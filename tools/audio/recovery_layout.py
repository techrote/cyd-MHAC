"""Read-only classic CYD candidate/stock evidence. Never opens serial or writes flash."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import struct
import zipfile

from verify_candidate import FLASH, APP, require, sha, inspect_image, check_partition

# Strict profile, not a generic ESP32 partition editor or uploader.
SLOTS = {'bootloader.bin': (0x1000, 0x7000), 'partitions.bin': (0x8000, 0x1000),
         'boot_app0.bin': (0xE000, 0x2000), 'firmware.bin': (0x10000, APP)}
BOOT_APP0_BLOB = '13562cabb9648287fdf70d2a22789fdf1e4156b4'
ROWS = [('nvs', 1, 2, 0x9000, 0x5000, 0), ('otadata', 1, 0, 0xE000, 0x2000, 0),
        ('app0', 0, 0x10, 0x10000, APP, 0), ('spiffs', 1, 0x82, 0x310000, 0xE0000, 0),
        ('coredump', 1, 3, 0x3F0000, 0x10000, 0)]


def check_files(files, offsets=None):
    require('firmware.bin' in files and set(files) <= set(SLOTS), 'missing app or unknown image')
    images = {}
    for name, data in files.items():
        offset, capacity = SLOTS[name]
        require(0 < len(data) <= capacity, 'image exceeds slot: ' + name)
        images[name] = {'offset': offset, 'bytes': len(data), 'sha256': sha(data)}
        if name in ('firmware.bin', 'bootloader.bin'):
            images[name]['image'] = inspect_image(data)
        elif name == 'partitions.bin':
            table = check_partition(data)
            rows = [tuple(e[k] for k in ('name', 'type', 'subtype', 'offset', 'size', 'flags')) for e in table['entries']]
            require(rows == ROWS, 'not the exact huge_app partition profile')
            images[name]['table'] = table
        else:
            require(len(data) == 0x2000, 'unexpected boot_app0 size')
            blob = hashlib.sha1(b'blob 8192\0' + data).hexdigest()
            require(blob == BOOT_APP0_BLOB, 'boot_app0 differs from reviewed Arduino 2.0.17 blob')
    if offsets is not None:
        require(all(type(v) is int for v in offsets.values()) and offsets == {name: item[0] for name, item in SLOTS.items()}, 'unexpected uploader offsets')
        require(set(files) == set(SLOTS), 'incomplete resolved uploader images')
    return {'images': images, 'missing': sorted(set(SLOTS) - set(files)),
            'resolved_offsets_verified': offsets is not None,
            'status': 'HOLD: offline integrity only; not flash approval'}


def inspect_archive(path, source, digest):
    require(path.stat().st_size <= 32 * 1024 * 1024, 'oversize archive')
    raw = path.read_bytes()
    require(sha(raw) == digest, 'archive SHA-256 mismatch')
    with zipfile.ZipFile(io.BytesIO(raw)) as z:
        names = z.namelist()
        require(len(names) == len(set(names)), 'duplicate ZIP names')
        require(z.getinfo('source-sha.txt').file_size <= 128 and z.getinfo('SHA256SUMS').file_size <= 8192, 'oversize receipt')
        actual = z.read('source-sha.txt').decode('ascii').strip()
        require(re.fullmatch('[0-9a-f]{40}', actual) and actual == source, 'source SHA mismatch')
        receipt = {'source_sha': actual, 'archive_sha256': digest}
        manifest = [line.split() for line in z.read('SHA256SUMS').decode('ascii').splitlines() if line.strip()]
        files = {}
        for name, (_, capacity) in SLOTS.items():
            if name not in z.namelist():
                require(not any(row[-1] == name for row in manifest), 'manifest references missing image')
                continue
            require(z.getinfo(name).file_size <= capacity, 'oversize ZIP member: ' + name)
            data = z.read(name)
            require([row for row in manifest if row[-1] == name] == [[sha(data), name]], 'image manifest mismatch: ' + name)
            files[name] = data
        offsets = None
        if 'upload-layout.json' in z.namelist():
            require(z.getinfo('upload-layout.json').file_size <= 32768, 'oversize layout receipt')
            layout = json.loads(z.read('upload-layout.json'))
            require(isinstance(layout, dict), 'invalid layout receipt')
            offsets = layout.get('offsets')
            require(isinstance(offsets, dict), 'missing resolved offsets')
        return {**receipt, **check_files(files, offsets)}


def stock_pair(first, second):
    require(not first.samefile(second), 'use two independent backup files')
    require(first.stat().st_size == second.stat().st_size == FLASH, 'both backups must be exactly 4 MiB')
    a, b = first.read_bytes(), second.read_bytes()
    require(len(a) == len(b) == FLASH and a == b, 'backup bytes differ or size changed')
    require(a != b'\xff'*FLASH and a != b'\0'*FLASH, 'blank backup')
    return {'bytes': len(a), 'sha256': sha(a), 'status': 'MATCH only; provenance, security and restorability unverified'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)
    archive = commands.add_parser('archive')
    archive.add_argument('path', type=Path)
    archive.add_argument('--source', required=True)
    archive.add_argument('--archive-sha256', required=True)
    stock = commands.add_parser('stock-pair')
    stock.add_argument('first', type=Path)
    stock.add_argument('second', type=Path)
    args = parser.parse_args()
    try:
        result = (inspect_archive(args.path, args.source, args.archive_sha256) if args.command == 'archive'
                  else stock_pair(args.first, args.second))
    except (OSError, ValueError, KeyError, TypeError, struct.error, zipfile.BadZipFile) as exc:
        parser.exit(1, f'FAIL: {exc}\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
