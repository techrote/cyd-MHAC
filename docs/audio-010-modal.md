# CYD-AUDIO-010 — Modal Chime (opt-in v3)

Refs #1, cumulative Draft/HOLD #2. Exact parent 080ea47fb8073f126e40f7d2beb40d6a97cf2eda; previous audio qualification 45782897996 succeeded. Visual HEAD ecf5d452927712d91a9ad4f24440d27f37df07c6 was read-only. No new dependencies or reused upstream code. Diviner platform/license at 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC README at 495a98ca12bae467a507031f04931aa4254b1866 and FM-1-MHAC README at 16c698d9e244559064a9a9a22f834d36912bf498 reread; same platform and evidence boundary. Creative struck-instrument approximation, not an empirical MHAC implementation.

Four damped sinusoidal modes at ratios 1, 2.01, 3.97, 5.43. X retains pitch mapping, Y varies brightness and damping. Gate rising excites over 64 samples; hold lets the sound decay; release then touch restrikes. No simulated input or microphone required. Fixed state, bounded mix, no per-sample heap, I/O, locks or transcendental functions beyond the existing engine. Crossfade and global gate/watchdog retained. Host sizeof Engine 3244 bytes vs 3196 before; no physical timing claim.

The legacy six-mode enum, v1/v2 publication and 12 factory presets are unchanged. Explicit Exchange::publish_modal(x,y,gate,mute) publishes ID 6 with bit 29 set; raw ID 6 without bit 29 still falls back to PhaseMorph, and v2 publish_patch(6) still sanitizes to zero. v3 consumers opt in only after pinning this commit; no automatic visual UI changes. MHPS v1 intentionally cannot store ID 6. Serial diagnostic key 9 selects ephemeral Modal Chime; p replays selected sound, 0/m release or mute, and r recalls a valid legacy sound. Saving modal is rejected without altering NVS. No persistence format change.

Native commands (run in local recovered exact source plus new files):
    g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tests/modal_test.cpp -o /tmp/modal_test
    /tmp/modal_test raw
    clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude tests/modal_test.cpp -o /tmp/modal_test_san
    /tmp/modal_test_san

Both PASS: nine pitch/brightness pairs, decay, restrike, exact split equivalence, bounds, 3000 rapid switches, watchdog silence/rearm and v2 invalid-ID fallback. Independent local differential used parent engine/feature headers (blobs 7e163a1c04cdbd520336e13fe66239bde472f763 / 19cfc3bea8810998508d1dbef794118c08ca020a) from AUDIO-008 archive: 12000 mixed legacy blocks, 772464 exactly matching samples/control/analysis per GCC and Clang sanitized run. New tested blobs engine a131b86c03ad7c5e813551ee331a940bfa78634e, modal ef110b7194cccc94fb8635acb0d9008ff823f229, test 1eab6beafb6caf9a255c0c0a900d7c93ef5a535b. Native rendering produced nine raw PCM16 specimens, 24 kHz mono, three seconds each. No listening claim.

CI adds one test to existing workflow. Local Git DNS and PlatformIO unavailable, so changed serial/target build is pending CI. Hardware gate remains: measure CPU/underruns, validate GPIO26 playback and touch integration, evaluate modal sound subjectively. No main merge or flash approval. Visual lane may retain six modes until explicitly adopting v3; do not change shared files concurrently.
