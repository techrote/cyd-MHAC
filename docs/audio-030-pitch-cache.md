# CYD-AUDIO-030 — exact stationary-pitch cache

Refs #1, cumulative Draft/HOLD #2. Parent 2a1ecb1073fdcfd04edc8de3b14257de6fbf9972, qualified by run 45939030884. Integrated VISUAL-028 c815221763c09ff84d516b0559cf521d1d6ac6a4 run 45941321680: both jobs SUCCESS; Windows artifact 15173047624, target 15173261489 unexpired and exact source per artifact metadata. New audio candidate not yet target-qualified.

Cache the pitch target in Engine using the accepted 12-bit X code, retaining exactly the existing exp2 expression when X changes. Y-only moves, stationary held/latch renewals, and partitioned rendering no longer recalculate exp2 for the same pitch. No control/analysis contract, sample rate, oscillator math, presets, hardware configuration, or allocation changes. Engine native size increases 3436 to 3444 bytes; recompile clients. This removes unnecessary transcendental calls by inspection, not a measured ESP32 speedup. First call uses an out-of-range cache sentinel. The cache is owned solely by the audio thread and cannot see an interrupted writer's unaccepted word.

Local GCC and Clang optimized C++17, plus Clang ASan/UBSan: exact 2,541,248 samples with matching DAC words, 28,545 feature reads, packed analysis, nine intentional watchdog trips, all 4096 pitch codes in ascending/descending order, Y-only moves, 31/97 partitions, mode transitions and interrupted control. All five binary receipts (old/new on both compilers and new sanitizer) match SHA256 44e93c6b16cc076b94bb5acfcb32f8ba4f8b5984d87f494db112464a4416a3b5. Hash is a local result, not a platform-independent oracle. Tested source blobs: header 9b13e4c2fe8c353bf3e40a555bbcb791ca1aaf35, test 9c4281d22a4d2944e96a555208c8cc162bc650aa; five original headers verified at parent.

Reproduce by extracting the five audio headers (mhac_audio, mhac_dac, mhac_features, mhac_modal, mhac_quotation) from the parent into old/ and using the new include/:
```sh
g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iold tools/audio/pitch_cache_check.cpp -o /tmp/pitch-old
g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tools/audio/pitch_cache_check.cpp -o /tmp/pitch-new
/tmp/pitch-old > /tmp/old.bin
/tmp/pitch-new > /tmp/new.bin
cmp /tmp/old.bin /tmp/new.bin
```
Repeat with clang++; sanitizer flags -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer. Existing audio CI gains invariant execution and a diagnostic hash, no new jobs. Local target toolchain absent; real target deadlines, physical device, flashing and independent owner audition remain HOLD. Diviner HAL/MIT, MHAC README, FM-1 README and Trace Bell source reread at recorded exact pins; no upstream code copied. No main merge. Next: integrated owner audition and physical measurement rather than further cosmetic host-only expansion.
