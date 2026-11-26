#!/usr/bin/env python3
"""Offline review of CYD-MHAC VISUAL-016 plain serial telemetry; never opens a port."""
import argparse
import json
import re
from pathlib import Path

U32 = 2**32 - 1
RATE, BLOCK = 24000, 128
BUDGET = (BLOCK * 1000000 + RATE - 1) // RATE
FIELDS = {
    'visual': ('frame_us', 'strip_us', 'step', 'blocks', 'errors', 'watchdog'),
    'audio': ('prep_us', 'render_us', 'over', 'failed'),
}
PATTERNS = {k: re.compile(k + ''.join(r' ' + f + r'=([0-9]{1,10})' for f in fs))
            for k, fs in FIELDS.items()}


def review(lines, min_blocks=5625):
    if not 1 <= min_blocks <= U32:
        raise ValueError('min_blocks out of range')
    first, last = {}, {}
    counts = dict.fromkeys(FIELDS, 0)
    invalid, flags = set(), set()
    boots = 0
    max_frame = max_strip = 0
    steps = set()
    for line in lines:
        line = line.strip()
        if line.startswith('MHAC audio initialization FAILED'):
            flags.add('audio initialization failure')
        if line.startswith('MHAC audio initialized;') or line.startswith('MHAC audio initialization FAILED'):
            boots += 1
            if boots > 1 or last:
                invalid.add('restart or concatenated sessions')
        kind = line.partition(' ')[0]
        if kind not in FIELDS:
            continue
        match = PATTERNS[kind].fullmatch(line)
        if match is None:
            invalid.add('malformed ' + kind + ' row')
            continue
        row = dict(zip(FIELDS[kind], map(int, match.groups())))
        if any(v > U32 for v in row.values()):
            invalid.add('out-of-range ' + kind + ' row')
            continue
        if kind == 'visual':
            monotonic = ('blocks', 'errors', 'watchdog')
            if row['step'] not in (2, 4):
                invalid.add('unknown visual quality step')
            max_frame = max(max_frame, row['frame_us'])
            max_strip = max(max_strip, row['strip_us'])
            steps.add(row['step'])
            for key in ('errors', 'watchdog'):
                if row[key]:
                    flags.add(key + ' nonzero')
        else:
            monotonic = FIELDS[kind]
            if row['render_us'] > row['prep_us']:
                invalid.add('render maximum exceeds preparation maximum')
            for key in ('over', 'failed'):
                if row[key]:
                    flags.add(key + ' nonzero')
            if row['prep_us'] >= BUDGET:
                flags.add('preparation maximum reaches block budget')
        if kind in last and any(row[k] < last[kind][k] for k in monotonic):
            invalid.add(kind + ' lifetime counter/maximum decreased (reset or wrap)')
        first.setdefault(kind, row)
        last[kind] = row
        counts[kind] += 1
    for kind in FIELDS:
        if counts[kind] < 2:
            invalid.add('need two or more ' + kind + ' rows')
    blocks = (last['visual']['blocks'] - first['visual']['blocks']) if 'visual' in last else 0
    if blocks < min_blocks:
        invalid.add('insufficient completed-block span')
    if invalid:
        verdict = 'INSUFFICIENT'
    elif flags:
        verdict = 'HOLD_INDICATORS'
    else:
        verdict = 'CLEAR_OBSERVED_INDICATORS'
    return {
        'verdict': verdict, 'rows': counts, 'startup_markers': boots,
        'issues': sorted(invalid), 'flags': sorted(flags),
        'first': first, 'last': last, 'block_span': blocks,
        'minimum_blocks': min_blocks,
        'nominal_audio_seconds': round(max(0, blocks) * BLOCK / RATE, 3),
        'preparation_budget_us': BUDGET, 'max_observed_frame_us': max_frame,
        'max_observed_strip_us': max_strip, 'observed_quality_steps': sorted(steps),
        'limitations': 'Silent-window snapshots only; no wall-clock duration, true DMA underruns, physical audio, or source identity established. CLEAR is not hardware acceptance.',
    }


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('log', type=Path, help='one plain-text serial log, one reboot/phase')
    p.add_argument('--min-blocks', type=int, default=5625, help='minimum completed-block span (default 5625 = 30 nominal audio seconds)')
    args = p.parse_args()
    try:
        with args.log.open(encoding='utf-8', errors='strict') as f:
            result = review(f, args.min_blocks)
    except (OSError, UnicodeError, ValueError) as exc:
        p.error(str(exc))
    print(json.dumps(result, indent=2, sort_keys=True))
    return {'CLEAR_OBSERVED_INDICATORS': 0, 'HOLD_INDICATORS': 1, 'INSUFFICIENT': 2}[result['verdict']]


if __name__ == '__main__':
    raise SystemExit(main())
