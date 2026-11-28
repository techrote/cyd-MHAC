# CYD-VISUAL-007 — Gesture quotation and timing telemetry

Refs #3/#4. Previous visual 72ae9aff6de451bc372c1520b393324409f11b2a, run 45762184783 SUCCESS. Pinned AUDIO-007 25cd21b4d43f8f58db823a49c6e4faf04a047683, run 45768143939 SUCCESS. Original UI-owned code; no new third-party reuse. Existing Diviner MIT notice retained; re-read HAL at 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC README at 495a98ca12bae467a507031f04931aa4254b1866 and FM-1-MHAC README at 16c698d9e244559064a9a9a22f834d36912bf498. No scientific claim.

Top-centre 24x24 fresh-touch region arms a single-gesture quotation when unmuted; amber=armed, red=recording, teal=playing, grey=idle. Drag after releasing the arm control. On release a gesture >=32ms repeats with a 100ms gate-off rest. Up to 4000ms, one point at least 16ms apart, 251 entries max; a longer gesture is truncated but playback waits for release. Coordinates are the already smoothed and scale-mapped audio controls, replayed with zero-order hold; not audio samples, not sample-accurate automation. A fresh touch stops playback and returns control to the player; touching the loop button stops without playing a note. Mute, stale held input, watchdog and transport fault cancel the loop. No auto-unmute, no new audio contract, no callback allocations. Original touch Surface remains unchanged. Effective controls drive both audio and frame. No external input assumed.

Audio timing v1 read on UI task, alternate low-rate silent diagnostic lines at 115200. prep_us/render_us are lifetime wall-time maxima, over is preparation budget exceedances, failed is failed/short driver writes. Not underruns, DAC timestamps or CPU utilization. No heartbeat coupling.

Local isolated native commands (files copied to one directory):
    g++ -std=c++17 -O2 -Wall -Wextra -Werror -I. gesture_test.cpp -o /tmp/gesture && /tmp/gesture
    clang++ -std=c++17 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -I. gesture_test.cpp -o /tmp/gesture_san && /tmp/gesture_san

Both PASS, 1516 bytes native, 10000 sessions, bound, wrap, cancellation, rest. Tested blob IDs: header ebdea9013ab180247cde629b638c33b076c109fa, test 678ad3b25b0c49c1acbf922e0d84e003b3c9f198. Integrated performance test uses actual audio Engine and visual renderer; configured in existing visual workflow, not locally executed. Local Git DNS blocked and SDL2 development / PlatformIO unavailable. Require exact-head target and host CI, owner touch calibration and physical audition; no flash approval. Next interactive native preview and physical stress qualification.
