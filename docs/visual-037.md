# VISUAL-037 — gesture-lit routing glyphs (Refs #3)

Draft/HOLD. Visual parent 245029a3b7c11ba80e1ee9400e4b62e925d4b9f1, run 46000136987 both jobs SUCCESS. Import exact AUDIO-037 b09158a8e7d09037e78818cc3f3778f9fb41d612, run 46007033855 SUCCESS, four audio-owned blobs unchanged. Previous qualified Windows artifact 15196734899 and target artifact 15196949757 confirmed nonexpired and matching visual parent by metadata, not downloaded.

Replace existing Glyph mode 2 with a stable 20 by 15 lattice of 16-pixel cells. Deterministic unsigned hashes define shared horizontal and vertical edge occupancy and four interior motifs. Geometry does not reseed between frames. Gesture position originates decorative travelling Manhattan-distance illumination; timbre controls spatial wave density, peak controls brightness, sampled phase and tick drive pulse timing. No signal propagation model, decoding, perceptual evidence or new audio dependency. Preserve palettes, markers, mute, patch/mode counts and existing gesture overlay. No additional persistent state or allocation; only integer math. Device cost and adaptive-quality appearance remain to be measured.

Executed native standalone test:
    g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/glyph_study.cpp -o glyph && ./glyph frame-
    clang++ -std=c++17 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude tests/glyph_study.cpp -o glyph-san && ./glyph-san san-

Both PASS 9040 shared boundary comparisons (686 active) and 460800 deterministic bounded RGB565 pixels. Six distinct hashes, byte-identical PPMs between compilers. Synthetic controls, not integrated or hardware captures. Tested blobs a4804f26531061d1c2840cbcdb1cf8513ba8bd65 and e7a012c9e8085b2e8eb2fa0303c7c00836a14c85. Hashes a703d31afc9f4861 f1e601618eff0ef3 1b7c22eaf3c55969 52984474a2e1ff53 5476c29484155760 fce727e299353378. Existing integrated native replay tests cover mode 2 and three quality tiers; new standalone test added to existing job. Integrated and Windows/ESP32 qualification pending; local DNS blocks Git and PlatformIO unavailable.

Read-only reference inspection: Diviner MIT, theme.cpp, hal_cyd.cpp, hal_native.cpp at 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README 495a98ca12bae467a507031f04931aa4254b1866; FM-1 README 16c698d9e244559064a9a9a22f834d36912bf498; Trace Bell README d996a0b8c457630e3d18a4b2a06393640f8d3fbf. No upstream code copied. No main merge or flash approval. Next: integrated qualification, physical audition and rendering cost.
