"""Read-only same-source CYD firmware/Windows delivery verification (no device access)."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import struct
import zipfile

SLOTS = {'bootloader.bin': (0x1000, 0x7000), 'partitions.bin': (0x8000, 0x1000),
         'boot_app0.bin': (0xe000, 0x2000), 'firmware.bin': (0x10000, 0x300000)}
INNER = 'CYD-MHAC-Windows-x64.zip'
WINDOWS = {'desktop-loop.ppm', 'desktop-smoke.txt', 'mhac-desktop.exe', 'Open CYD-MHAC.cmd',
           'README.txt', 'sdl-source-sha.txt', 'SDL2-LICENSE.txt', 'source-sha.txt', 'visual-diviner-license.txt'}


def require(ok, message):
    if not ok:
        raise ValueError(message)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def archive(data, expected):
    require(len(data) <= 32*1024*1024 and sha(data) == expected, 'archive digest/size mismatch')
    with zipfile.ZipFile(io.BytesIO(data)) as z:
        entries = z.infolist()
        names = [e.filename for e in entries]
        require(len(names) == len(set(n.casefold() for n in names)), 'duplicate archive path')
        require(all(re.fullmatch(r'[A-Za-z0-9_. /-]+', n) and all(p not in ('', '.', '..') for p in n.split('/')) for n in names), 'unsafe archive path')
        require(sum(e.file_size for e in entries) <= 32*1024*1024, 'archive expansion limit')
        return {n: z.read(n) for n in names}


def manifest(files, name, expected):
    rows = [line.split(None, 1) for line in files[name].decode('ascii').splitlines() if line.strip()]
    require(len(rows) == len(expected) and all(len(r) == 2 for r in rows), 'manifest shape')
    require({r[1] for r in rows} == expected, 'manifest coverage')
    require(all(r[0] == sha(files[r[1]]) for r in rows), 'manifest digest mismatch')


def verify(firmware, windows, source, firmware_digest, windows_digest):
    require(re.fullmatch('[0-9a-f]{40}', source), 'expected exact source SHA')
    fw = archive(firmware, firmware_digest)
    outer = archive(windows, windows_digest)
    require(all(x['source-sha.txt'].decode('ascii').strip() == source for x in (fw, outer)), 'source mismatch')
    manifest(fw, 'SHA256SUMS', set(SLOTS))
    layout = json.loads(fw['upload-layout.json'])
    require(layout['resolved_offsets_verified'] is True and layout['missing'] == [], 'incomplete layout')
    require(layout['offsets'] == {n: s[0] for n, s in SLOTS.items()} and set(layout['images']) == set(SLOTS), 'layout shape')
    for n, (offset, capacity) in SLOTS.items():
        item = layout['images'][n]
        require(0 < len(fw[n]) <= capacity and (item['offset'], item['bytes'], item['sha256']) == (offset, len(fw[n]), sha(fw[n])), 'layout image mismatch: '+n)
    manifest(outer, 'SHA256SUMS.txt', {INNER})
    inner = archive(outer[INNER], sha(outer[INNER]))
    require(set(inner) == WINDOWS | {'SHA256SUMS.txt'}, 'unexpected portable package members')
    manifest(inner, 'SHA256SUMS.txt', WINDOWS)
    require(inner['source-sha.txt'].decode('ascii').strip() == source, 'inner source mismatch')
    require(inner['sdl-source-sha.txt'] == outer['sdl-source-sha.txt'], 'SDL source mismatch')
    exe = inner['mhac-desktop.exe']
    pe = struct.unpack_from('<I', exe, 0x3c)[0]
    require(exe[:2] == b'MZ' and exe[pe:pe+6] == b'PE\0\0\x64\x86' and exe[pe+24:pe+26] == b'\x0b\x02', 'not AMD64 PE32+')
    return {'source': source, 'firmware_archive_sha256': firmware_digest, 'windows_archive_sha256': windows_digest,
            'portable_zip_sha256': sha(outer[INNER]), 'firmware_images': layout['images'],
            'portable_files_verified': len(WINDOWS), 'status': 'HOLD: delivery integrity only; no physical audition or flash approval'}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('firmware', type=Path)
    p.add_argument('windows', type=Path)
    p.add_argument('--source', required=True)
    p.add_argument('--firmware-sha256', required=True)
    p.add_argument('--windows-sha256', required=True)
    a = p.parse_args()
    try:
        require(all(x.stat().st_size <= 32*1024*1024 for x in (a.firmware, a.windows)), 'archive size')
        result = verify(a.firmware.read_bytes(), a.windows.read_bytes(), a.source, a.firmware_sha256, a.windows_sha256)
    except (OSError, ValueError, KeyError, TypeError, struct.error, zipfile.BadZipFile) as e:
        p.exit(1, f'FAIL: {e}\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
