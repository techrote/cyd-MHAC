# VISUAL-026 — Six session performance scenes (Refs #3)

Draft/HOLD. Visual parent 03c3303d4477c8b5346b8a431b2224657252ab8a (45923888842 both jobs SUCCESS), audio parent e568fa7917611c6f66136f97424f9c1e74abe241 (45927183276 host-and-target SUCCESS). The three AUDIO-027 qualification-only blobs are imported byte-for-byte. No audio runtime, driver, patch, storage or interface change.

The six green/grey bars at x40–87, y216–239 cycle a session scene. One tap atomically changes the sound, visual, palette, XY mapping, scale and starting coordinates; selection consumes the entire contact, preserves mute, cancels gesture looping and clears the factory cursor. Individual controls remain available; if the tuple no longer matches a scene, the next tap selects Lantern Tide. The 16 sound-only factory entries are unchanged; audition continues to use that independent bank. Scene names appear in the desktop title. No extra persistent runtime state, framebuffer or storage format; compile-time table and indicator checks only. Original creative combinations, not new DSP or scientific/perceptual results.

| Scene | Sound | Visual | Palette | Mapping / scale |
|---|---|---|---|---|
| Lantern Tide | Harmonic Lattice | Diffusion | Verdigris | Note lattice / minor pentatonic |
| Cinder Clock | Erasure Pulse | Cellular | Ember | Rhythm bands / chromatic |
| Silver Bells | Modal Chime | Chime | Silver | Note lattice / minor pentatonic |
| Memory Crossing | Counterquotation | Palimpsest | Original | Direct / chromatic |
| Afterwater | Recursive Echo | Ink | Verdigris | Fine relative / free |
| Dust Prism | Ring Dialect | Caustic | Ember | Swapped / free |

Local qualification on 2026-11-27, repo-root commands:
    g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tests/scene_test.cpp -o scene
    ./scene scene-
    clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude tests/scene_test.cpp -o scene-san
    ./scene-san

Both PASS: 1152 selector pixels without overlap, 432 RGB565 overlay checks across 1/2/4 tiers, 238324 active PCM samples, bounded output, mute/loop/watchdog interlocks. Six actual rendered RGB565 frames (virtual touch and actual synthesis, not wall-clock/device captures) have matching cross-compiler hashes f2ba11f4c72051b2 / eda93e6be909fa9e / 7ed7057f749e080a / a2d9427a2018fc21 / bb6522b40f70646f / 0e20b28ce5afd604. Prior Diffusion, Quotation, Cellular, XY mapping and Caustic regressions rerun PASS. The original archived 22 headers were verified against the exact visual parent; changed tested blobs: visual ce60677ebb4a969b9db8ff5352351be4701089ee, performance 9bd4ef806357ffd1daeaca4a60cb522aa96128fb, scenes a6d051b302ec44f589dcd5b241c68ff60a635e31, test bd9962ea3ab3583a96641c5619c9210602620e5a. Existing CI job extends with test and six PPM captures; desktop title change and actual target compile require integrated CI. Local Git DNS unavailable.

Read-only references reviewed: Diviner HAL and MIT license 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README 495a98ca12bae467a507031f04931aa4254b1866; FM-1-MHAC README 16c698d9e244559064a9a9a22f834d36912bf498; VST group routing 550085862590cd51419001ad5cba39a78b0ba387. No upstream code copied. Hardware configuration and existing attribution unchanged.

Latest fully qualified previous Windows artifact 15165796329 and ESP32 artifact 15165334073 (VISUAL-025), metadata confirmed unexpired; no archives independently downloaded this run. No physical audio, touch or timing measurement, no flash approval, no main merge. Next: owner audition and actual device timing; refine gesture functionality after those findings.
