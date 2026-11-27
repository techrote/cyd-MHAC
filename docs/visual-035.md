# VISUAL-035 — chronological gesture trajectories (Refs #3)

Draft/HOLD. Visual parent 9863b2bd0273824f5728b74ea49e711774f53c41; qualified AUDIO-035 3e179d4fc8913b3846c12549a60b390141d7e6e4 (run 45990347215 SUCCESS); import its four changed audio-owned blobs by exact identity. VISUAL-034 run 45983264616 SUCCESS; Windows artifact 15190332192 and target artifact 15190357233 nonexpired/source-matched metadata, not independently downloaded. Prior visual stack retained.

Original full-resolution dashed links between the existing 16 sampled recording coordinates (at most 15 segments). Earlier links warm, later links cooler; beads remain above links. Clip the integer sample-index range to each eight-row strip before plotting. No new persistent or framebuffer memory, no audio path change, no heap, no floating point in rasterizer. Each segment at most 320 candidate samples over a full frame; no full-line rescan per strip. Respect existing mute, cursor and rhythm-legend exclusion, performance area and all quality tiers. Recorded-order decimated path, not an exact trajectory, raw touch, or audio telemetry. UI test adds a deterministic endpoint-link witness and synthetic control-coordinate RGB565 capture in the existing Linux job.

Executed standalone exact geometry source locally:
```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/trajectory_test.cpp -o test && ./test
clang++ -std=c++17 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude tests/trajectory_test.cpp -o test-san && ./test-san
```
Both PASS: 2000 seeded profiles including horizontal, vertical, reversed and coincident endpoints; 117125 plotted samples compared with a full-segment reference; exact 8-row strip/full-frame equivalence, bounds. Local tested blobs 7cb66b61a743ee8ee11f7d737409c25aaea19de6 and 430f1ffbf7b6da40e62be43c0d274dfcbbe02210. Integrated renderer/capture, Windows and ESP32 compile await CI; no physical performance or audio deadline claim. No new jobs or dependencies.

Read-only references checked: Diviner 82e06e11e5947e41e888716e7a68e99187410f2e theme, CYD/SDL HAL and MIT notice; MHAC 495a98ca12bae467a507031f04931aa4254b1866 README; FM-1 16c698d9e244559064a9a9a22f834d36912bf498 README; Trace Bell 408af1d6c2a3fdc2a444bb36f44ac0b2ce44be64 VST3 docs. No upstream code copied; creative exaptation, not a scientific protocol or perception claim. No main merge or flashing. Next: inspect exact CI capture and owner physical measurements.
