# AUDIO-021 — modal phase-increment cache (Refs #1, #2, #4)

Parent 2c97d0248d336ac4cce81a5f3f7085731b18f6b3. Cache the four integer phase increments per voice, refreshing when the quantized pitch step changes. Released voices retain their last step. No change to pitch, phase accumulation, excitation, decay, gain, gate/lease, preset formats or wire contract. 40 extra bytes in native Engine (3316 -> 3356; ModalChime 112 -> 152); clients must rebuild. No heap, locks, I/O or new dependency. Rejected additional decay-coefficient caching: slower in the native stress benchmark.

## Executed 2026-11-26

All seven baseline headers matched the pinned Git blob IDs. GCC 14.2 optimized and Clang 17 ASan/UBSan candidate: 16 factory audition recordings and all 48 PCM/DAC/legacy waveform hashes unchanged from baseline; each checks alternate 128/31/97 sample partitions. New modal_cost.cpp exercises 3 x 24000 128-sample blocks (9,216,000 PCM words per run). Exact rolling hash agreement across baseline and optimized GCC and candidate Clang sanitizer, bounded PCM and zero unexpected watchdog trips. These are hash comparisons, not exhaustive proofs.

Median of three runs' median block wall times (ns; host Linux x86, unpaced, no DAC/hash in timed interval):

| Scenario | Old | New |
|---|---:|---:|
| Held fixed pitch (includes silent tail) | 1753 | 1711 |
| Held moving pitch (includes silent tail) | 1786 | 1710 |
| Repeated short strikes, overlapping tails | 6964 | 6095 |

AUDIO-020 render_cost stress: modal 7519 -> 7026 ns; mixed 3798 -> 3809 ns; all eight scenario hashes unchanged. Additional decay cache was rejected (modal median 8709 ns). Timing tails are scheduler-sensitive. No target-speed inference.

Reproduce in a checkout: extract parent include directory to separate directory with git archive, then compile tools/audio/modal_cost.cpp, tools/audio/gesture_audition.cpp and tools/audio/render_cost.cpp with g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -I<include-dir> <source> -o <exe>. Run baseline/candidate; compare hashes and repeat timings. Sanitized candidate modal_cost and gesture_audition: clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude <source> -o <exe>. No local PlatformIO or physical device measurement.

Tested blobs: modal header 43a92b839515947a9843be9a131e1fbad6b85bd6; benchmark 56cc4303b5f120d7011699be74151212d3fcb432.

AUDIO-020 run 45882510137 and VISUAL-019 run 45884984560 both completed successfully. Exact integrated 9f696c5c580d4b6e13f57b8c56d16128a99cea8b Windows artifact 15148354990, ESP32 artifact 15148788984; live listing only, not locally downloaded. Both remain prior candidates, not AUDIO-021. New CI gate outstanding. Read-only reference README refs: Diviner 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC 495a98ca12bae467a507031f04931aa4254b1866, FM-1-MHAC 16c698d9e244559064a9a9a22f834d36912bf498. No upstream code reused. Draft/HOLD. Physical audition, stock backup/recovery, actual audio continuity and owner acceptance outstanding.
