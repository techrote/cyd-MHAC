# CYD-VISUAL-040 — Persistent diffusion canvas (Refs #3, #1)

Draft/HOLD. Parent 0a1a863b7459318b137d5795a7317ee092caed11, integrated run 46025973500 both jobs SUCCESS. Audio #2 remains 83616abc38fe60cf1817023d4233a7aa9a411522 (run 46024660174 SUCCESS), unchanged. The two same-source integrated artifacts are Windows 15211373704 (archive SHA256 5f1b8a5651efbf68a6dd2a41e8d62c44a15502059a0cac19b2e7f0a2770d10a6), target 15212715312 (archive SHA256 bda244fd402e1d83b79b9abce75513fa7e79e6c40e188546197b63232b6016ce), both nonexpired per API. These qualify the parent, not this increment; archive digests are API metadata, not locally recomputed.

Diffusion now retains its two-species field when deselected, freezes its clock, and resumes without accumulating missed steps. Held gestures paint connected paths between observed fixed-step seed coordinates using at most eight stamps per step; no bridge across an observed release/mute/deselection. The field is session-only, not flash persistence. Rapid events entirely between visual observations are not guaranteed to be seen. Original simulation, aggregate peak modulation, shading and audio contract unchanged. Added four bytes on tested native ABI (6164 B total; existing 6176 B bound). No audio path, new framebuffer, allocation, dependency, pin, or task. Bounded extra visual computation, physical cost not yet measured.

Native commands (from repository root):
    g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/diffusion_memory_test.cpp -o /tmp/study && /tmp/study gcc-
    clang++ -std=c++17 -O1 -g -fsanitize=address,undefined -Wall -Wextra -Werror -Iinclude tests/diffusion_memory_test.cpp -o /tmp/study-san && /tmp/study-san clang-

Both executed PASS: 24/24 bridged and 0/24 lifted interior cells, 251 exact grid comparisons (192768 cell comparisons), 98304 cell bounds. Four 320x240 RGB565-quantized shade-only PPM captures byte-identical across compilers. Synthetic coordinates/peak, not full UI or hardware captures. Tested candidate header blob d5b1d16a678bda9783d1be50b79f2ae144498edb; test blob 428b8bc30328dc0390b5951dd5c9694ca84d8a77. Existing full integrated diffusion study retained; new CI regression added to existing job, no extra jobs. Integrated Windows/target qualification outstanding for new head.

References revisited read-only: Diviner HAL and MIT license at 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README 495a98ca12bae467a507031f04931aa4254b1866; FM-1-MHAC README 16c698d9e244559064a9a9a22f834d36912bf498. Original artwork mechanism, no upstream code copied. No scientific claims. No main merge or physical flash approval.

Next visual: exact-head integrated tests, profile worst-case fast strokes and owner audition; next audio: substantial new synthesis or deadline evidence, retaining AUDIO-039. Physical board identity, recovery backup, concurrent deadlines and audition remain HOLD.
