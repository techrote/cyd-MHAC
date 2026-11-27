# CYD-AUDIO-012 — bounded overlapping Modal Chime strikes

Refs #1 and cumulative Draft/HOLD #2. Exact parent 6cc2810c4eace5d4960facb48be64569c5d1eddb. AUDIO-011 host/target run 45800105732 SUCCESS; VISUAL-011 exact 976ae65734d30474e9234e1760e0eb63027c83af run 45802582746 both Windows and Linux/ESP32 jobs SUCCESS. Windows artifact 15107492775 is the prior seven-mode instrument, not this revision. Main 6549f280672a5eb5cf9478e32f002454599d0c40 only bootstrap; AGENTS.md/RAG.md absent on main and audio parent.

Two fixed four-mode voices, half gain each, bounded sum. New contact strikes a quiet slot or fades the quieter occupied slot over 64 samples (2.67 ms at 24 kHz) before reuse. Released voices retain pitch and brightness while current contact follows smoothed XY. Explicit mute, watchdog expiry and patch deselection inhibit excitation and accelerate damping. Global envelope still applies: this is short overlapping rearticulation, not an independently gated polyphonic keyboard or free-running long release. Single voice level is intentionally lower (half gain); no automatic gain compensation. No added dependencies, heap, I/O, locks or publication contract change; 12 factory sounds and MHPS v1 unchanged. Modal native state 104 bytes vs 48; Engine 3308 vs 3244 (64 bytes increase including alignment). Physical timing unknown.

Native commands:
    g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tests/modal_test.cpp -o /tmp/modal12
    /tmp/modal12 raw
    clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude tests/modal_test.cpp -o /tmp/modal12san
    /tmp/modal12san

Both PASS: direct two-voice pitch lock and additivity for 1024 samples; independent nonzero tail/new energy 77.653/28.588 (sum of squared normalized samples); 12000 constant-phase amplitude continuity checks under repeated stealing; inhibit and clean rearm; nine pitch/brightness combinations, exact split invariance, decay/restrike, 3000 switches, watchdog and v2 fallback. Independent GCC and Clang-sanitized local differential against AUDIO-008 recovered reference also PASS 12000 legacy blocks, 772464 exact samples each. Local tested engine/modal/test blobs b0f20d3f35b72c7acbc9e9e9501e6705923c0a46 / afae5d72104694b34920801073b519ed880bb6e8 / d6e49ef55b6c892ad7a545a06ba8de723ae59ff6. Source recovered from AUDIO-010 audition archive and parent blob identity checked. Existing CI already compiles modal_test.cpp, no workflow change or new jobs. Local DNS prevents git clone, no physical board test.

Diviner exact 82e06e11e5947e41e888716e7a68e99187410f2e platformio.ini, HAL and MIT license reviewed; MHAC 495a98ca12bae467a507031f04931aa4254b1866 README and FM-1 16c698d9e244559064a9a9a22f834d36912bf498 README read-only. Original musical exaptation, not direct MHAC protocol or perceptual evidence; no upstream code reused.

Next: exact target qualification, visual lane explicit pin only after CI, physical audio deadline and owner audition, complete flash recovery kit. No main merge, no flash approval.
