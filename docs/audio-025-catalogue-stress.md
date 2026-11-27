# AUDIO-025 — complete session-catalogue transition qualification (Refs #1, #2, #3)

Exact parent eac65c554bbaf2e2189ad3728cba078ae34d77b3; AUDIO-024 run 45910185648 SUCCESS. VISUAL-023 exact 430b75f35f5d0babe86bcf25066ad7c938dc11b4 run 45912115932 SUCCESS (both Windows and target jobs); unexpired Windows artifact 15160541901 and target artifact 15161120796 metadata identify that exact head. No download/re-hash of those archives in this increment.

New host-only catalogue_stress.cpp exercises all 81 ordered pairs of the nine session sounds, including nine self-pairs; each dwell is 160 blocks (20,480 samples, sufficient to fill the quotation history), with varying X and high Y, late short release/rearm and 48 muted tail blocks. Two independent Engine/Exchange/DacQuantizer instances use identical control publication boundaries but 128 versus variable three-part rendering. Compare each PCM sample, DAC word, quantizer residual and every field of the 128-sample feature window; verify correct sample clock, output bound, nonzero sound segments, final digital silence and zero unexpected watchdog trips. Nine self-pair hashes must differ. Packed block-local peak/sequence is intentionally not compared across partitions. This is not a claim that every transition is perceptually click-free. No DSP or hardware behavior changed.

Native Linux x86, executed 2026-11-27: GCC optimized and Clang ASan/UBSan both PASS, 81 pairs, 3,815,424 PCM comparisons, same number of DAC comparisons, 29,808 complete feature-window comparisons per compiler. Aggregate PCM+DAC hash e0bd70157b12dbfd both compilers. Test source blob dbce57b9d8ffc8d6c8c6f8b2703d8c7dd8300459; all five input headers recovered from AUDIO-024 archive matched exact parent Git blob IDs.

One unpaced GCC host timing observation per eligible steady-state block, 936 per mode. Median/p99 in ns: Phase Morph 3155/3916; Harmonic Lattice 3085/3565; Ring Dialect 2674/3075; Detuned Swarm 2564/3024; Erasure Pulse 3636/4336; Recursive Echo 2453/2694; Modal Chime 4547/4838; Quotation 3014/3245; Counterquotation 2994/3435. Timed region includes render and DAC packing, not publishing, hashes, checks, audio transport or LCD. Host scheduler/CPU noise applies; do not extrapolate to ESP32 or claim a physical deadline. No hard timing threshold in test. Modal Chime remains the highest observed median in this fixture.

Commands from root:
```sh
g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tools/audio/catalogue_stress.cpp -o catalogue_stress
./catalogue_stress
clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tools/audio/catalogue_stress.cpp -o catalogue_stress_san
./catalogue_stress_san
```

Read-only pinned reference review: Diviner src/hal_cyd.cpp and MIT LICENSE at 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README at 495a98ca12bae467a507031f04931aa4254b1866; FM-1-MHAC README at 16c698d9e244559064a9a9a22f834d36912bf498. No imported source. Existing CI job now runs the new harness; no new jobs, dependencies, interface, firmware or visual-owned changes. Local PlatformIO absent; target CI pending. Draft/HOLD; no physical performance, board/recovery, flash or main merge approval. Next: use the expanded evidence alongside physical telemetry and owner audition; avoid further speculative headroom claims.
