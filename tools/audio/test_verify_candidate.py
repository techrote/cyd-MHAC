"""Synthetic partition fixtures; real image controls run separately in CI."""
import hashlib
import struct
import unittest
from verify_candidate import check_partition

ROWS = [(1, 2, 0x9000, 0x5000, b'nvs'), (1, 0, 0xE000, 0x2000, b'otadata'),
        (0, 0x10, 0x10000, 0x300000, b'app0'), (1, 0x82, 0x310000, 0xE0000, b'spiffs'),
        (1, 3, 0x3F0000, 0x10000, b'coredump')]


def fixture(rows):
    body = b''.join(struct.pack('<HBBII16sI', 0x50AA, *r, 0) for r in rows)
    body += b'\xeb\xeb' + b'\xff'*14 + hashlib.md5(body, usedforsecurity=False).digest()
    return body.ljust(0xC00, b'\xff')


class PartitionTests(unittest.TestCase):
    def test_expected_layout(self):
        self.assertEqual(check_partition(fixture(ROWS))['app_capacity'], 0x300000)
        self.assertEqual(check_partition(fixture(ROWS).ljust(0x1000, b'\xff'))['app_offset'], 0x10000)

    def test_corruption(self):
        data = fixture(ROWS)
        for bad in (data[:-1], data[:20]+bytes([data[20]^1])+data[21:], data[:-1]+b'\0', b'\xff'*0xC00):
            with self.subTest():
                with self.assertRaises(ValueError):
                    check_partition(bad)

    def test_valid_md5_invalid_layout(self):
        # Recompute MD5 deliberately: integrity alone cannot establish safe bounds.
        for row in [(1, 0x82, 0x300000, 0xE0000, b'spiffs'),
                    (1, 0x82, 0x310001, 0xE0000, b'spiffs'),
                    (1, 0x82, 0x310000, 0x100000, b'spiffs'),
                    (1, 0x82, 0x310000, 0xE0000, b'nvs')]:
            with self.subTest(row=row):
                with self.assertRaises(ValueError):
                    check_partition(fixture(ROWS[:3]+[row]+ROWS[4:]))
        with self.assertRaises(ValueError):
            check_partition(fixture(ROWS[:2]+[(0, 0x10, 0x10000, 0x200000, b'app0')]+ROWS[3:]))


if __name__ == '__main__':
    unittest.main()
