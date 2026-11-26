# AUDIO-024 — Counterquotation (Refs #1, #2, #3)

Parent 8e7282ab43735253bf112893b7a78c394bac6f6b (AUDIO-023 run 45904063733 SUCCESS). VISUAL-022 exact 2a5f64afb58afc0b8da758ff41d4313321f007e9 run 45906269554 SUCCESS, Windows artifact 15158470317 and target 15158008447; metadata confirms exact head and unexpired. These preceding binaries do not contain this change.

Original creative exaptation: a second Quotation interpretation retains the actual four stored gesture marks, alternately recalling them an octave below (even ring index) and a perfect fifth above (odd ring index). Capture/replay timing remains 3000 active samples per mark, first recall at 12000 samples, no frame-clock dependence. Pitch changes glide using existing slew, no phase reset or added voice/state allocation. Switching Quotation variants during an active gate changes the target at the next recalled mark, retains current history; mute, watchdog and switching away from both variants retain existing invalidation rules. X remains live pitch; Y remains blend depth. No claim of a formal semantic protocol, perception, or device performance.

Optional v5: Exchange::publish_counterquotation(x,y,gate,mute) sets wire ID 7, bits 30 and 31, bit 29 clear. Bit 31 alone with ID 7 is not a valid opt-in; all three flags with ID 7 likewise fall back to PhaseMorph. v1–v4 publishers remain unchanged. Session selection index 8 is NOT a wire ID or persistent preset ID; mix_patch_count remains 8. Visual worker must explicitly add a ninth session selection and dispatch publisher, not increment mix_patch_count; existing 16 factory presets/codecs unchanged. Weak serial UI T selects the session, p repeats, m mutes; 2-second timeout unchanged. Serial path not locally compiled; target CI required. Strong visual UI overrides weak hooks.

Executed 2026-11-26 Linux x86 using five exact parent header blobs and existing test blob recovered from AUDIO-023 evidence and matched against GitHub. GCC 14.2 and Clang 17 ASan/UBSan PASS: 48 observed recalled oscillator increment checks (3 pitch steps x 2 variants x 8 recalls, tolerance 63 phase-step units); no premature recall, bounds; new full-depth six-second render hash f09e00e7cc6fa80c; 128/31/97 sample partition exact; dry equals legacy, bit31-only and conflicting flags fall back; existing 24 rapid-rearm profiles and watchdog tests PASS. Existing three audition WAVs byte-identical to AUDIO-022. Engine 3436 native bytes unchanged. Tested candidate blobs: audio ad14d8ee2437a435358db74d0d4c8378dbd52dd0, quotation 6b01b8670d5c3dd96ace1c725a1450c4bb35be41, study dfd8fd3ec53ec72c18efd9f71c309edadd2117e6. New WAV SHA256 01c8f7f491c731004ee87f1fc180009047008faf974179fad6e3966b05440592.

Commands:
```sh
g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tools/audio/quotation_study.cpp -o quote
./quote dry.wav half.wav wet.wav counter.wav
clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tools/audio/quotation_study.cpp -o quote-san
./quote-san
```

Read-only pinned README refs: Diviner 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC 495a98ca12bae467a507031f04931aa4254b1866, FM-1-MHAC 16c698d9e244559064a9a9a22f834d36912bf498. No upstream code reused. No new CI jobs, dependency or visual-owned changes. Existing workflow runs the extended test. Local target toolchain not used; no listening, physical timings, flash/main merge approval. Next: exact-head target CI, visual dispatch and paired audition, then physical gate.
