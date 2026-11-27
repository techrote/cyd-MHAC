# CYD-AUDIO-032 — terminal fault evidence in the integrated instrument

Refs #1; cumulative Draft/HOLD PR #2. Exact parent f697fde5f9c937cc1d83f35426aecfdf2e414678, run 45950859597 host-and-target SUCCESS. Integrated VISUAL-030 848e31e03e84501fe33e9e442e40be2e7bb30d12 run 45953193233 both jobs SUCCESS. Windows artifact 15176853879 and target artifact 15176909918 nonexpired per metadata; no independent download or archive rehash in this increment.

VISUAL-030 stops ordinary telemetry once it presents its terminal fault screen. AUDIO-031's explicit terminal serial marker was emitted only from the weak diagnostic UI (overridden by the visual lane). Consequently a log with adequate earlier clean rows could end at an actual fault without a marker, and the offline reviewer did not recognize even the weak UI marker. Move the one-shot report into the common Arduino loop after UI tick, retaining weak/strong mute handling and audio fault counter semantics. No logging in audio_task, no changes to DSP, pins, DMA or shared control layout. One loop-owned boolean and an atomic read per healthy tick; serial writes only upon terminal fault. Report is best-effort: a halted loop, incomplete capture, or disconnected serial monitor can still omit it. No inference of fault absence from silence.

The reviewer now recognizes the exact marker and retains the terminal flag even when the log is insufficient. Adequate previously clean logs with a terminal marker become HOLD_INDICATORS. Incomplete or restarted logs remain INSUFFICIENT, not clear. Older logs remain compatible. No changes to reviewer thresholds or timing interpretation.

Executed local:
```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tools/audio/fault_notice_test.cpp -o /tmp/notice-gcc
/tmp/notice-gcc
clang++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Werror -Iinclude tools/audio/fault_notice_test.cpp -o /tmp/notice-san
/tmp/notice-san
cd tools/audio && python -m unittest -v test_fault_telemetry.py
```
Both native tests PASS (fault at startup or after 1/100 healthy ticks; 1000 subsequent polls per case); Python three methods PASS (five insertion positions, four incomplete/reboot cases, duplicate/whitespace, clean baseline). Tested blob IDs: fault helper 9a4728cbb28f429e589b0cb57ec00295dbd88d74; native test 7a325f2943b286c926cca8a7b81c6eceba56d8be; reviewer 77f05ffadf2a1638748c47a86ac7c19815e9fdfa; Python test b6cea37f2c5e21b31e7e0020cfcf09eeb1d6d3c4. Existing CI job gains both telemetry suites and native notice regression. Actual Arduino integration and physical capture not locally executed (PlatformIO absent; GitHub DNS unavailable from local shell). No hardware, flash or main merge approval.

Read-only reference review: Diviner README/MIT 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC README 495a98ca12bae467a507031f04931aa4254b1866, FM-1 README 16c698d9e244559064a9a9a22f834d36912bf498, Trace Bell state 4282017bcb560e552c4e25cf88a62bbbfd83f56b. No upstream code reused. Next: use exact qualified integrated build for owner recovery/physical audition, verify marker capture and timing evidence on target.
