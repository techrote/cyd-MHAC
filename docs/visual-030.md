# VISUAL-030 — terminal audio fault screen (Refs #3)

Draft/HOLD. Integrate exact AUDIO-031 f697fde5f9c937cc1d83f35426aecfdf2e414678 (run 45950859597 SUCCESS) into VISUAL-029 2d47432011dba01f6b3c67a89f9652ebb2c36d36 (run 45946441585 SUCCESS). Five audio-owned changes imported unchanged from 1297d4584cf1927363fb813f5ae0948841d0b327; no shared contract or audio code edits by visual lane.

On the first terminal audio error, the device cancels calibration and loop state, forces mute, publishes once, and resets the display strip cursor. A high-contrast fixed RGB565 screen reads AUDIO FAULT / REBOOT, independent of palette, gesture, calibration, adaptive step or analysis. Thirty existing eight-row transfers render one static frame; after that the terminal path returns without further touch polling, animation, or SPI transfers. No new framebuffer, heap, task, hardware pin or DSP. Normal operation and the recoverable watchdog remain separate. Fault detection is polled; a fault during a strip is observed on a subsequent tick, not asynchronously. Physical display, startup/write failure injection and actual response time not tested. No remote flash or main merge approval.

Local executed on reconstructed VISUAL-029 headers (unchanged audio DSP):
```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/fault_screen_test.cpp -o /tmp/fault-gcc && /tmp/fault-gcc
clang++ -std=c++17 -O1 -g -fsanitize=address,undefined -Wall -Wextra -Werror -Iinclude tests/fault_screen_test.cpp -o /tmp/fault-san && /tmp/fault-san
```
Both PASS: 76,800 raster pixels (4,900 red, 1,359 title, 891 reboot, 69,650 background); strip sentinels, playing-loop cancellation and maintained mute. Tests exercise renderer and UI forceMute, not physical HAL or the target tick adapter. Tested blob IDs: header 5aec2d63ee4afb37d771334a9db59295c87504d1; test 4acbb3bd1c603ea0ec141695d73e401ccbad28d6; unchanged visual e140cb40d74c5e3df3362e5b7662b191e5b5f18e; audio 9b13e4c2fe8c353bf3e40a555bbcb791ca1aaf35. Existing workflow adds regression without new jobs. New integrated target/Windows build pending.

Read-only pinned reference README and MIT license Diviner 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README 495a98ca12bae467a507031f04931aa4254b1866; FM-1-MHAC README 16c698d9e244559064a9a9a22f834d36912bf498. No upstream code imported. Prior qualified Windows artifact 15175166477 and target artifact 15175022153 remain unexpired per API; not independently downloaded this run.
