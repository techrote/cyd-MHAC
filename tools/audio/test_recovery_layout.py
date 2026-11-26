"""Offline synthetic fixtures and fake PlatformIO env; no device access."""
import hashlib
import json
from pathlib import Path
import runpy
import struct
import tempfile
import unittest
import zipfile

from recovery_layout import SLOTS, ROWS, check_files, inspect_archive, stock_pair
from verify_candidate import FLASH, sha


def image():
    header = bytearray(24)
    header[0:4] = bytes([0xE9, 1, 2, 0x20])
    header[23] = 1
    body = bytes(header) + struct.pack('<II', 0x40080000, 4) + b'\x01\x02\x03\x04'
    body += b'\0' * (15 - len(body) % 16) + bytes([0xEF ^ 1 ^ 2 ^ 3 ^ 4])
    return body + hashlib.sha256(body).digest()


def table(rows=ROWS):
    body = b''.join(struct.pack('<HBBII16sI', 0x50AA, kind, subtype, offset, size, name.encode(), flags)
                    for name, kind, subtype, offset, size, flags in rows)
    body += b'\xeb\xeb' + b'\xff'*14 + hashlib.md5(body, usedforsecurity=False).digest()
    return body.ljust(0xC00, b'\xff')


def boot_init():
    # Minimal exact-byte fixture verified by the reviewed upstream Git blob hash.
    data = bytearray(b'\xff'*8192)
    data[:4] = b'\x01\0\0\0'
    data[28:32] = bytes([154, 152, 67, 71])
    data[4096:4100] = b'\0'*4
    return bytes(data)


class RecoveryTests(unittest.TestCase):
    def setUp(self):
        self.files = {'firmware.bin': image(), 'bootloader.bin': image(),
                      'partitions.bin': table(), 'boot_app0.bin': boot_init()}
        self.offsets = {name: item[0] for name, item in SLOTS.items()}

    def test_profile(self):
        self.assertEqual(check_files(self.files, self.offsets)['missing'], [])
        self.assertEqual(check_files({'firmware.bin': image()})['missing'],
                         ['boot_app0.bin', 'bootloader.bin', 'partitions.bin'])
        for name in self.files:
            with self.subTest(name=name), self.assertRaises(ValueError):
                check_files({k: v for k, v in self.files.items() if k != name}, self.offsets)
            with self.subTest(offset=name), self.assertRaises(ValueError):
                check_files(self.files, {**self.offsets, name: self.offsets[name] + 1})
            with self.subTest(corrupt=name), self.assertRaises(ValueError):
                data = self.files[name]
                check_files({**self.files, name: data[:-1] + bytes([data[-1] ^ 1])}, self.offsets)
        with self.assertRaises(ValueError):
            check_files({**self.files, 'bootloader.bin': b'\0'*0x7001}, self.offsets)
        with self.assertRaises(ValueError):
            check_files({**self.files, 'partitions.bin': table([('nvs', 1, 2, 0x9000, 0x4000, 0)] + ROWS[1:])}, self.offsets)

    def test_archives(self):
        source = 'a'*40
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'receipt.zip'
            def write(files, layout=None, manifest=None):
                with zipfile.ZipFile(path, 'w') as z:
                    z.writestr('source-sha.txt', source + '\n')
                    z.writestr('SHA256SUMS', manifest if manifest is not None else ''.join(f'{sha(v)}  {k}\n' for k, v in files.items()))
                    for name, data in files.items():
                        z.writestr(name, data)
                    if layout is not None:
                        z.writestr('upload-layout.json', json.dumps(layout))
                return sha(path.read_bytes())
            digest = write(self.files, {'offsets': self.offsets})
            self.assertTrue(inspect_archive(path, source, digest)['resolved_offsets_verified'])
            for bad_source, bad_digest in [('b'*40, digest), (source, '0'*64)]:
                with self.assertRaises(ValueError):
                    inspect_archive(path, bad_source, bad_digest)
            digest = write(self.files)
            self.assertFalse(inspect_archive(path, source, digest)['resolved_offsets_verified'])
            digest = write(self.files, {'offsets': {**self.offsets, 'boot_app0.bin': 0x9000}})
            with self.assertRaises(ValueError):
                inspect_archive(path, source, digest)
            manifest = ''.join(f'{sha(v)}  {k}\n' for k, v in self.files.items())
            for bad in [manifest + f'{sha(self.files["bootloader.bin"])}  bootloader.bin\n',
                        manifest.replace(sha(self.files['boot_app0.bin']), '0'*64)]:
                digest = write(self.files, manifest=bad)
                with self.assertRaises(ValueError):
                    inspect_archive(path, source, digest)

    def test_stock(self):
        with tempfile.TemporaryDirectory() as tmp:
            a, b = Path(tmp)/'a.bin', Path(tmp)/'b.bin'
            data = b'\xe9' + b'\xff'*(FLASH-1)
            a.write_bytes(data); b.write_bytes(data)
            self.assertEqual(stock_pair(a, b)['sha256'], sha(data))
            with self.assertRaises(ValueError):
                stock_pair(a, a)
            for bad in [data[:-1], data[:-1] + b'\0']:
                b.write_bytes(bad)
                with self.assertRaises(ValueError):
                    stock_pair(a, b)
            for blank in [b'\0'*FLASH, b'\xff'*FLASH]:
                a.write_bytes(blank); b.write_bytes(blank)
                with self.assertRaises(ValueError):
                    stock_pair(a, b)

    def test_capture(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            for name, data in self.files.items():
                (root/name).write_bytes(data)
            class Env:
                def subst(self, s):
                    return {'$PROJECT_DIR': str(root), '$BUILD_DIR': str(root), '$ESP32_APP_OFFSET': '0x10000'}.get(s, s)
                def get(self, key, default):
                    return [(hex(offset), str(root/name)) for name, offset in self.offsets.items() if name != 'firmware.bin']
                def AddCustomTarget(self, name, dependency, action, **kwargs):
                    self.action = action
            env = Env(); env.offsets = self.offsets.copy()
            runpy.run_path(str(Path(__file__).with_name('capture_layout.py')), init_globals={'Import': lambda _: None, 'env': env})
            env.action(None, None, env)
            receipt = json.loads((root/'layout-evidence/upload-layout.json').read_text())
            self.assertEqual(receipt['offsets'], self.offsets)
            self.assertEqual(receipt['missing'], [])
            self.assertEqual((root/'layout-evidence/boot_app0.bin').read_bytes(), self.files['boot_app0.bin'])
            env.offsets['boot_app0.bin'] = 0x9000
            with self.assertRaises(ValueError):
                env.action(None, None, env)


if __name__ == '__main__':
    unittest.main()
