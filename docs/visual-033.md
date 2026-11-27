# VISUAL-033 — independent pitch overdub (Refs #3)

Draft/HOLD. Visual parent c6c9a4ffd071db13f59d0bbe30f80bf123d6d2b2, integrated audio cf40af216f026589fa3a9d59361005e672a83880. Prior visual run 45966693726 both jobs success; Windows artifact 15183715945, ESP32 artifact 15183062095, source-matched metadata, nonexpired. Audio run 45964831719 success. No new audio import in this increment; no writes outside visual-owned paths. Existing gesture storage reused. Pitch edit uses a distinct cyan selector x240–263 y0–23, exclusive with amber timbre selector. Horizontal absolute pitch for Direct/Swapped/Fine/Rhythm, with scale quantization; exact note cell for Lattice. No insertion, time or gate changes; rest exclusion. Lift disarms, watchdog and mute cancel. Session-only destructive creative extension, not a formal MHAC protocol.

Local verified parent gesture blob 68c13b33ff184510180f2909576e863b7e63b1ed, candidate d78f75499fa7f709ea3aa19070bdf8976182a6f5, test 33c20e5719c7251e87791b59b9e81152a2232068. Executed:
```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/pitch_core_test.cpp -o core
./core
clang++ -std=c++17 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude tests/pitch_core_test.cpp -o core-san
./core-san
```
Both PASS: 23520 active edits, 6480 rest exclusions, 23520 changed pitch observations, exact timbre/gate/count/duration across six transports and two clock origins including rollover. Local Git DNS unavailable; core header reconstructed and blob matched. Integrated surface/DSP differential and new marker have not been executed locally; existing visual workflow extended with core and integration tests, no new jobs. No new target build, hardware, subjective listening or flash approval claimed. Next exact-head CI and owner audition; consider undo after physical memory/performance evidence.

Read-only Diviner 82e06e11e5947e41e888716e7a68e99187410f2e README/MIT, hal_cyd, theme, hal_native, visions; MHAC README 495a98ca12bae467a507031f04931aa4254b1866; FM-1 README 16c698d9e244559064a9a9a22f834d36912bf498; Trace Bell VST3 docs cb8752cb51d27f9076edb710964988a7fedb3cf7. No upstream code copied, new dependencies, pins, audio contract or main merges.
