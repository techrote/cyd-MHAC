import unittest
from telemetry_review import review

BOOT = 'MHAC audio initialized; serial 1-9'
FAULT = 'MHAC audio terminal fault; reboot required'
V = 'visual frame_us=40000 strip_us=2300 step=2 blocks={} errors=0 watchdog=0'
A = 'audio prep_us=1300 render_us=1100 over=0 failed=0'
BASE = [BOOT, V.format(100), A, V.format(5725), A]


class FaultTelemetryTests(unittest.TestCase):
    def test_complete_log(self):
        self.assertEqual(review(BASE)['verdict'], 'CLEAR_OBSERVED_INDICATORS')
        for i in range(1, len(BASE)+1):
            with self.subTest(offset=i):
                r = review(BASE[:i] + [FAULT] + BASE[i:])
                self.assertEqual(r['verdict'], 'HOLD_INDICATORS')
                self.assertIn('terminal audio fault', r['flags'])

    def test_truncated_and_reboot(self):
        for rows in ([FAULT], [BOOT, FAULT], BASE[:3] + [FAULT], BASE + [FAULT, BOOT]):
            with self.subTest(rows=rows):
                r = review(rows)
                self.assertEqual(r['verdict'], 'INSUFFICIENT')
                self.assertIn('terminal audio fault', r['flags'])

    def test_duplicate_and_whitespace(self):
        r = review(BASE + ['  ' + FAULT + '\r\n', FAULT])
        self.assertEqual(r['verdict'], 'HOLD_INDICATORS')
        self.assertEqual(r['flags'], ['terminal audio fault'])


if __name__ == '__main__':
    unittest.main()
