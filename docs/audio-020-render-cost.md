# CYD-AUDIO-020 — demand-gated oscillators (Refs #1, #2, #4)

Parent 02b91f1c99fda9c294903cc544ff052bdcbe009d. Post-transition weights determine which carrier and phase-modulated samples need evaluating. All phase clocks, detune updates, feature accumulation, envelopes, DAC state, patch IDs, public interface and patch storage remain unchanged. Skip the carrier for a fully modal mix and the modulation pair when neither Phase Morph nor Erasure Pulse contributes. No additional persistent state, allocations, I/O or dependencies. No upstream code reused.

## Native evidence, 2026-11-26

GCC 14.2.0 optimized: baseline and candidate full factory audition metrics identical (48 waveform hashes, 16 presets; each checks exact internal 128/31/97 partition equivalence, peak, mute and watchdog). Matches prior AUDIO-019 metrics. New render_cost.cpp exercises 12000 128-sample blocks per scenario, 7 individual engines plus all 49 ordered patch pairs, sweeping pitch and timbre, gate and mute. 12,288,000 PCM samples and corresponding DAC words per run; all eight hashes identical between old/new GCC and new Clang 17 ASan/UBSan. Peak <=8191 and watchdog zero. Hash identity is not a formal exhaustive proof. Tested new header blob 6a53a3c51d60c6c22d8a6be43626f39e8baf2fef; benchmark blob 28e6adb39a6fec1ef86b74dddeb3754804b28572. All seven baseline headers and audition source matched exact parent Git blob IDs.

Median of three runs' median block wall times (ns; includes DAC and rolling hashes, excludes publishing, allocation and output checks; unpaced Linux x86 host, not ESP32; scheduler noise affects tails):

| Scenario | Baseline | Candidate |
|---|---:|---:|
| Phase Morph | 3827 | 3807 |
| Harmonic Lattice | 4639 | 3777 |
| Ring Dialect | 4408 | 3176 |
| Detuned Swarm | 4318 | 3327 |
| Erasure Pulse | 4268 | 4388 |
| Recursive Echo | 4278 | 3096 |
| Modal Chime | 11251 | 8836 |
| All ordered patch pairs | 4499 | 3867 |

Reproduction: extract baseline include files with git archive 02b91f1c99fda9c294903cc544ff052bdcbe009d include into a separate directory, then from candidate root compile tools/audio/render_cost.cpp and tools/audio/gesture_audition.cpp separately using g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -I<baseline-or-candidate-include> <source> -o <exe>. Execute each without arguments; diff factory CSVs and first three columns of render_cost CSVs. Candidate sanitized benchmark compiled with clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tools/audio/render_cost.cpp -o render_cost_san. Run and compare first three columns. Native results only, no real-time or physical performance claims. Local git DNS unavailable, PlatformIO absent. Hosted target build is a separate gate.

AUDIO-019 run 45874817264 and VISUAL-018 run 45876881979 verified successful. VISUAL-018 exact 17996ee14a929afc688e55e4d2f4fbfb1d845bee; Windows artifact 15145148603, target artifact 15144454501. Artifacts listed, not downloaded in this increment. No physical audition, stock backup, recovery, flash or main merge approval.

Read-only reference review: Diviner README at 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README at 495a98ca12bae467a507031f04931aa4254b1866; FM-1-MHAC README at 16c698d9e244559064a9a9a22f834d36912bf498; Afterimage phase_memory.hpp at 9b717745daaf04b4e42bd462412947634e98671c; acceleration phase_balanced_host.py at 99fb8061447a44f53d4ddb88f3c6b87cdcc44b54. CYD README has no repository-wide license, AGENTS.md and RAG.md absent at main and audio parent. Next: exact-head CI, visual lane import only after green, physical performance comparisons using AUDIO-018 telemetry. Draft/HOLD.
