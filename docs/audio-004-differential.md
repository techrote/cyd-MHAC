# CYD-AUDIO-004 — predecessor differential qualification

Refs #1, #2, #4. Exact parent 7abdaa4ba2108abfa2a906a5fe9bc94e1185c782; visual candidate 0cb088eb9d0825bc4a14418530e90472c1978922. No DSP, target adapter, or packed-interface changes. No upstream code reused in this increment. Pinned original references remain in audio-contract-v1.md and audio-003-patches.md.

## Failure receipt and replacement gate

Run 45742159643 failed the hard-coded local-host PCM hash assertion in patch_test.cpp, skipping target compile. Its audio_test logged 127981 nonzero samples, hash 3bc7687125281e8b (runner image Ubuntu 26.04 20261122.9). Previous successful audio-002 run 45735658376 logged 127971, 0fc3c4848d848826 (runner image 20261116.88). Both differ from the original local hash 49d6545897c9e1c6. Those different environments are not a controlled comparison; no attribution to a specific compiler/libm defect is established.

The new regression compares every PCM sample, packed control/analysis and watchdog count with the unchanged original audio-002 header (source commit 1fec8bc503806513dbfb71103d6b1dda97a90e3f, blob 9b17e33ad666d3a152ade642c4e363d3a51cd0ef), copied by Git blob identity into tests/reference/audio_002.hpp. CI checks the fixture identity before compilation. The fixture is namespaced only during test compilation. No tolerance, hash allowlist or skipped checks. The older fixed hash is retained as a diagnostic receipt rather than a portable oracle. Four-mode partition equality, distinct hashes, invalid patch fallback, rapid switching, watchdog/rearm, publisher equivalence and output bounds remain tested.

## Local receipts

GNU C++ 14.2.0 and Clang 17.0.0 on this host:
```
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/patch_test.cpp -o /tmp/cyd-patch
/tmp/cyd-patch
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/legacy_compare.cpp -o /tmp/cyd-compare
/tmp/cyd-compare
# repeat both with clang++
clang++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tests/legacy_compare.cpp -o /tmp/cyd-san
/tmp/cyd-san
```
Both optimized compilers passed both suites; ASan/UBSan differential PASS. Each differential compared 172773 exact samples across boot, 1000-block original XY trajectory, watchdog trip, held restart rejection, rearm, varying controls/block sizes, mute and release. Both hashes 49d6545897c9e1c6, 127971 nonzero samples in the original trajectory. Negative control: separate temporary include with output gain 8191 replaced by 8000 rejected at sample 128 (34 vs 35). Header blob afe9c3357850123c1829370176345a9171c40f44 unchanged; locally tested comparator ac96443c22d90bf0db5851acd4543bc0919fb91d, patch test 95e8f8fb6cedeb058600c40e29c3271c207526d7. Local Git DNS blocked and PlatformIO absent; new exact-head hosted run remains gate.

Audio CI now targets main-base PRs only (avoids duplicate audio-target job on stacked visual PR #4), checks out exact PR head and archives compiler/source receipts, host results and firmware application binary with SHA256 if target build succeeds. Same pinned PlatformIO/toolchain; one standard runner. Visual lane should integrate this exact new audio commit and may run legacy_compare on its own pinned checkout. Application-only binary is not a recovery bundle, no flash authorization, physical audition, target timing or audio underrun qualification. Next: close CI and then add target diagnostics and musical expansion.
