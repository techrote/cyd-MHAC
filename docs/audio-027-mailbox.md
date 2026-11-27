# AUDIO-027 — concurrent analysis and interrupted control publication (Refs #1)

Parent d6fd9adb9c1c916afd82fdcd7784b408645dbb20; AUDIO-026 run 45922175247 SUCCESS. Integrated VISUAL-025 03c3303d4477c8b5346b8a431b2224657252ab8a run 45923888842 both jobs SUCCESS. Windows artifact 15165796329, target artifact 15165334073: unexpired and exact head confirmed in metadata, archives not downloaded this run. No physical measurements.

New native witness deliberately pauses the control writer with an odd heartbeat while rendering 200 blocks, checks one watchdog trip and digital silence, then completes the abandoned active control and verifies that it cannot rearm the expired gate. Explicit mute/rearm restores synthesis. Forced heartbeat rollover from 0xfffffffc through zero also accepts the new control. A separate FeatureMailbox test publishes 200000 correlated snapshots to two concurrent readers, verifies all accepted fields against their sequence-derived pattern, verifies rejected reads preserve the prior snapshot, and checks both readers converge to the last publication. No target scheduling guarantee; this is a native stress and deterministic fault injection, not TSan or formal verification. Counts vary with scheduling.

Executed locally 2026-11-27 from exact archived headers (all five Git blob IDs independently compared against parent):
- g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -pthread -Iinclude tools/audio/mailbox_stress.cpp -o mailbox_stress && ./mailbox_stress
- clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -pthread -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tools/audio/mailbox_stress.cpp -o mailbox_stress_san && ./mailbox_stress_san

Both PASS. GCC: 31400 accepted coherent reads, 706362 rejected reads; Clang: 341671 accepted coherent reads, 3426132 rejected reads. Each run used 200000 publications. Tested new source blob f7cec8e53019e5f20c9bd6364665ba276621f81b. Existing CI job extended by two commands, no new job. No runtime, DSP, patch, driver, control contract or visual-owned changes.

Read-only pinned upstream Diviner MIT license 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC README 495a98ca12bae467a507031f04931aa4254b1866, FM-1 README 16c698d9e244559064a9a9a22f834d36912bf498, VST acceleration three_plugin_host.py 550085862590cd51419001ad5cba39a78b0ba387. No imported code. No AGENTS.md or RAG.md at current CYD parent; README governs scope. Main remains README-only. Draft/HOLD; no main merge or physical flash approval. Next: physical owner telemetry, stock backup/recovery, paired audition.
