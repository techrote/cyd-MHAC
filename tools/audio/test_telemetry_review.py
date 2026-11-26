import unittest
from telemetry_review import review

BOOT = 'MHAC audio initialized; serial 1-9'
V = 'visual frame_us=40000 strip_us=2300 step=2 blocks={} errors={} watchdog={}'
A = 'audio prep_us={} render_us=1100 over={} failed={}'
BASE = [BOOT, V.format(100, 0, 0), A.format(1300, 0, 0),
        V.format(5725, 0, 0), A.format(1500, 0, 0)]


class ReviewTests(unittest.TestCase):
    def test_clear(self):
        r = review(BASE)
        self.assertEqual(r['verdict'], 'CLEAR_OBSERVED_INDICATORS')
        self.assertEqual(r['nominal_audio_seconds'], 30)
        self.assertEqual(r['preparation_budget_us'], 5334)
        self.assertEqual(r['last']['audio']['prep_us'], 1500)
        self.assertEqual(review(BASE[1:])['verdict'], 'CLEAR_OBSERVED_INDICATORS')

    def test_hold_controls(self):
        for row in (V.format(6000, 1, 0), V.format(6000, 0, 1),
                    A.format(1500, 0, 1), A.format(5334, 1, 0),
                    A.format(5334, 0, 0)):
            with self.subTest(row=row):
                self.assertEqual(review(BASE + [row])['verdict'], 'HOLD_INDICATORS')
        self.assertEqual(review([V.format(0, 1, 0), A.format(1200, 0, 0),
                                 V.format(6000, 1, 0), A.format(1200, 0, 0)])['verdict'], 'HOLD_INDICATORS')

    def test_insufficient_controls(self):
        controls = [BASE[:-1], BASE[:3], BASE + [BOOT], BASE + [V.format(5, 0, 0)],
                    BASE + [A.format(1200, 0, 0)], BASE + [A.format(1500, 0, 4294967296)],
                    BASE + ['audio prep_us=bad'], BASE + [V.format(6000, 0, 0).replace('step=2', 'step=3')],
                    BASE + [A.format(1000, 0, 0)], BASE + ['visual frame_us=10'],
                    BASE + ['audio prep_us=1600 render_us=1100 over=0 failed=0 EXTRA'],
                    [BASE[1], BASE[2], V.format(5724, 0, 0), BASE[4]],
                    BASE + [V.format(6000, 1, 0), V.format(7000, 0, 0)],
                    BASE + [A.format(6000, 1, 0), A.format(6000, 0, 0)]]
        for rows in controls:
            with self.subTest(rows=rows):
                self.assertEqual(review(rows)['verdict'], 'INSUFFICIENT')
        self.assertEqual(review(BASE, 5626)['verdict'], 'INSUFFICIENT')

    def test_adaptive_and_non_atomic(self):
        rows = [BASE[1], BASE[2], BASE[3].replace('step=2', 'step=4'), BASE[4]]
        r = review(rows)
        self.assertEqual(r['observed_quality_steps'], [2, 4])
        self.assertEqual(r['verdict'], 'CLEAR_OBSERVED_INDICATORS')
        self.assertIn('Silent-window', r['limitations'])

    def test_bad_minimum(self):
        for value in (0, -1, 2**32):
            with self.assertRaises(ValueError):
                review(BASE, value)


if __name__ == '__main__':
    unittest.main()
