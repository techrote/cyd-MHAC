"""Offline CYD app-candidate integrity gate; never opens a serial port or writes flash."""
import argparse
import hashlib
import io
import json
from pathlib import Path
import re
import struct
import zipfile

FLASH = 4 * 1024 * 1024
APP = 0x300000


def require(ok, message):
    if not ok:
        raise ValueError(message)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def inspect_image(data):
    require(24 <= len(data) <= APP, 'application size')
    require(data[0] == 0xE9 and 1 <= data[1] <= 16, 'image magic or segment count')
    require(data[2] <= 3 and data[3] >> 4 == 2 and (data[3] & 15) in (0, 1, 2, 15), 'not a classic 4 MiB flash header')
    require(struct.unpack_from('<H', data, 12)[0] == 0, 'not ESP32 chip ID 0')
    require(data[23] == 1, 'expected appended SHA-256')
    pos, checksum, segments = 24, 0xEF, []
    for _ in range(data[1]):
        require(pos + 8 <= len(data), 'truncated segment header')
        addr, size = struct.unpack_from('<II', data, pos)
        pos += 8
        require(size % 4 == 0 and addr + size <= 2**32 and pos + size <= len(data), 'invalid segment extent')
        for byte in data[pos:pos+size]:
            checksum ^= byte
        segments.append({'address': hex(addr), 'bytes': size})
        pos += size
    end = (pos + 16) // 16 * 16
    require(end + 32 == len(data), 'truncated or unexpected trailing data')
    require(not any(data[pos:end-1]), 'nonzero image padding')
    require(data[end-1] == checksum, 'segment XOR checksum mismatch')
    require(hashlib.sha256(data[:end]).digest() == data[end:], 'embedded SHA-256 mismatch')
    return {'bytes': len(data), 'sha256': sha(data), 'chip_id': 0, 'flash_bytes': FLASH,
            'entry': hex(struct.unpack_from('<I', data, 4)[0]), 'segments': segments,
            'xor': hex(checksum), 'embedded_sha256': data[end:].hex()}


def read_archive(path, expected_source, expected_digest):
    raw = path.read_bytes()
    require(expected_digest is not None and sha(raw) == expected_digest, 'archive SHA-256 mismatch or missing expected digest')
    with zipfile.ZipFile(io.BytesIO(raw)) as z:
        require(len(z.namelist()) == len(set(z.namelist())), 'duplicate ZIP names')
        require(z.getinfo('firmware.bin').file_size <= APP, 'oversize application member')
        require(z.getinfo('source-sha.txt').file_size <= 128 and z.getinfo('SHA256SUMS').file_size <= 8192, 'oversize receipt')
        source = z.read('source-sha.txt').decode('ascii').strip()
        require(re.fullmatch('[0-9a-f]{40}', source) and source == expected_source, 'source SHA mismatch')
        data = z.read('firmware.bin')
        matches = [line.split() for line in z.read('SHA256SUMS').decode('ascii').splitlines() if line.strip()]
        app_hashes = [fields[0] for fields in matches if len(fields) == 2 and fields[1] == 'firmware.bin']
        require(app_hashes == [sha(data)], 'application manifest mismatch')
    return data, {'source_sha': source, 'archive_sha256': sha(raw)}


def check_partition(data):
    require(len(data) in (0xC00, 0x1000), 'unexpected partition table size')
    entries = []
    for pos in range(0, 0xC00, 32):
        row = data[pos:pos+32]
        if row[:2] == b'\xeb\xeb':
            require(row[2:16] == b'\xff'*14 and hashlib.md5(data[:pos], usedforsecurity=False).digest() == row[16:], 'partition MD5 mismatch')
            require(data[pos+32:] == b'\xff'*(len(data)-pos-32), 'unexpected partition trailer')
            break
        magic, kind, subtype, offset, size, label, flags = struct.unpack('<HBBII16sI', row)
        require(magic == 0x50AA and size > 0 and offset >= 0x9000 and offset + size <= FLASH, 'invalid partition extent')
        require(offset % (0x10000 if kind == 0 else 0x1000) == 0 and size % 0x1000 == 0, 'partition alignment')
        require(not any(offset < e['offset']+e['size'] and e['offset'] < offset+size for e in entries), 'overlapping partitions')
        name = label.split(b'\0', 1)[0].decode('ascii')
        require(name and not any(e['name'] == name for e in entries), 'empty or duplicate partition name')
        entries.append(dict(name=name, type=kind, subtype=subtype, offset=offset, size=size, flags=flags))
    else:
        raise ValueError('missing partition MD5 marker')
    apps = [e for e in entries if e['type'] == 0]
    require(len(apps) == 1 and apps[0]['subtype'] == 0x10 and apps[0]['offset'] == 0x10000 and apps[0]['size'] == APP and apps[0]['flags'] == 0, 'not expected huge_app layout')
    return {'sha256': sha(data), 'entries': entries, 'app_offset': apps[0]['offset'], 'app_capacity': APP}


def negative_tests(data):
    cases = [data[:-1], data+b'\0', b'\0'+data[1:]]
    for offset in (1, 2, 3, 12, 23, 28, 32, len(data)-33, len(data)-1):
        bad = bytearray(data)
        bad[offset] ^= 0x80
        cases.append(bytes(bad))
    for bad in cases:
        try:
            inspect_image(bad)
        except ValueError:
            continue
        raise ValueError('negative control unexpectedly accepted')
    return len(cases)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('candidate', type=Path, help='firmware.bin or qualification ZIP')
    p.add_argument('--source', help='exact expected Git SHA (required for ZIP)')
    p.add_argument('--archive-sha256', help='expected archive digest from GitHub (required for ZIP)')
    p.add_argument('--partitions', type=Path, help='optional generated partitions.bin')
    p.add_argument('--negative-tests', action='store_true')
    args = p.parse_args()
    try:
        if args.candidate.suffix.lower() == '.zip':
            data, result = read_archive(args.candidate, args.source, args.archive_sha256)
        else:
            data, result = args.candidate.read_bytes(), {}
        result['image'] = inspect_image(data)
        if args.partitions:
            result['partitions'] = check_partition(args.partitions.read_bytes())
        if args.negative_tests:
            result['rejected_negative_controls'] = negative_tests(data)
        result['status'] = 'PASS: integrity only; no flash or hardware approval'
    except (OSError, ValueError, KeyError, zipfile.BadZipFile, struct.error) as error:
        p.exit(1, f'FAIL: {error}\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
