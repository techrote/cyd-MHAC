# CYD-AUDIO-013 — independent modal release

Refs #1, cumulative Draft/HOLD #2. Exact parent ef6d848ae605b4b596d05c7ef663e1d822153ddc; prior host and ESP32 CI run 45809203156 SUCCESS. Visual #4 remains independently owned; no v3 wire, factory, storage, dependencies or workflow change.

Modal Chime now rings after an ordinary lift rather than inheriting the six legacy modes' global gate envelope. Existing modal excitation, two fixed voices, fade-before-steal, brightness-dependent damping, fixed half gain and 256-sample patch crossfade are retained. The modal contribution has its own bounded amplitude; nonmodal modes retain the previous output arithmetic exactly when modal weight is zero. Explicit mute, lease expiry on held contact and patch deselection latch fast damping on each existing modal voice until it is reset by a fresh strike. A released contact may decay naturally without continued UI publication; this is not a sustained gate or indefinitely looping tail. Visuals should use measured peak/envelope rather than assume touch-up implies immediate silence. This is original artistic exaptation, not a formal MHAC protocol or evidence of perception.

Local commands from archive recovered source with base header/test blob identities matching the pinned parent:
```sh
g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tests/modal_test.cpp -o /tmp/modal13
/tmp/modal13
clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude tests/modal_test.cpp -o /tmp/modal13san
/tmp/modal13san
g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tools/audio/modal_release_audition.cpp -o /tmp/release13
/tmp/release13 natural > natural.raw
/tmp/release13 mute > mute.raw
```
Both test compilers PASS: independent post-lift tail energy 1699876285 (sum squared PCM in 2048 samples), one-block mute then ordinary released control cannot resurrect slow decay, watchdog expiry and fresh rearm, 1024 overlap-additivity samples, 12000 stealing continuity samples, 9 pitch/brightness pairs, exact split invariance, 3000 patch switches, v2 fallback. Archive legacy differential (AUDIO-008 reference) separately PASS under GCC and sanitized Clang, 12000 blocks / 772464 exact PCM samples each. Engine sizeof 3316 (prior 3308); ModalChime 112 (prior 104), host only.

Matched three-strike audition from actual optimized C++ 24000 Hz mono, 96000 PCM16 samples per render. Prior last nonzero 0.548 s, new natural last nonzero 2.626917 s, new one-block mute at 0.512 s last nonzero 0.526750 s. RMS during 0.65–0.80 s: prior 0, natural 727.401 PCM units, muted 0. Peaks respectively 4991, 5587, 5587. PCM SHA256: prior 27c0512853380d518f837f1628f80862908b35069baa3077d420f4f9a13c2870; natural 8a6d9c145ed51f5f29c3a7cef55a131aa635bfbbefce6b5651cdb41a26d9e7da; mute 1a2c8fba5e6c394b91b9597c4401951fbbfa53be677247de76cecea0d14e4b8a. No listening claimed. Prior comparison compiled same audition source against exact parent headers.

Read-only refs reviewed: Diviner 82e06e11e5947e41e888716e7a68e99187410f2e platform, HAL and MIT license; MHAC 495a98ca12bae467a507031f04931aa4254b1866 README; FM-1-MHAC 16c698d9e244559064a9a9a22f834d36912bf498 README; Three Winters foundry 91dc53aca2076665693cbef9f0c61fd913b28745 live.hpp. No upstream code reused. No new physical tests or ESP32 compilation locally. Existing CI will run full host and target checks. Physical audition, deadline measurement and complete recovery packaging remain HOLD. Do not flash or merge.
