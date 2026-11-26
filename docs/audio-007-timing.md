# CYD-AUDIO-007 — task timing instrumentation

Refs #1, cumulative Draft/HOLD PR #2. Parent 7cee1c23814af28ced4c3d380fd1bf2fb704ed64; visual dependency reference 72ae9aff6de451bc372c1520b393324409f11b2a. Prior audio run 45761002091 and visual run 45762184783 both verified successful on 2026-11-26. Audio parent build log reports 24732 bytes static RAM, 287193 bytes app flash, firmware SHA256 549544e403dab15e9e611226c7880c31523594ed087cb6609e368a1c00767a52. PlatformIO framework 4.20017.260907+sha.dcc1105b, toolchain Xtensa 8.4.0+2021r2-patch5. No SDK, board, pin, oscillator, patch, control or feature contract changes.

## Optional timing v1 interface

Read `mhac::AudioTiming t{}; if (mhac::audio_timing.read(t)) { /* display or log on UI task */ }` from mhac_platform.hpp. No mandatory consumer. False leaves previous data unchanged; true may repeat. All counters are lifetime, modulo 2^32; maxima persist until reboot. Single audio task producer, all payload words atomic, sequential consistency and bounded one-attempt reads. Sequence wrap ABA would require 2^31 publications during one read. 44-byte native meter and 40-byte native mailbox, fixed allocation. The task samples esp_timer_get_time four times per DMA write attempt (before render, after render, after PCM packing, after blocking write); no per-sample timing, I/O, allocation or logging added to render. Existing failure/disable path unchanged. Failed write is included in the last timing snapshot before task shutdown. Old audio_blocks/audio_errors remain independently readable; do not require atomic agreement with timing.

- attempts / failed_writes: attempted full-block writes and non-OK or short writes.
- max_render_us: wall elapsed engine render; includes preemption.
- max_prepare_us: render plus DAC packing and small task overhead before write.
- max_write_us: elapsed blocking driver call, including normal pacing.
- max_start_gap_us: elapsed between adjacent render starts (no first-block gap).
- prepare_over_budget: preparation time >= ceil(128 * 1000000 / 24000) = 5334 us.
- long_start_gaps: start-to-start gap >= 5334 us. This can occur in NORMAL DMA pacing; it is not a deadline or underrun counter.

Timestamp subtraction uses unsigned modulo 2^32; consecutive measurements must be less than one full wrap (about 71.6 min). These are task timing and coarse budget indicators, NOT DMA occupancy, true underruns, CPU utilization or DAC timestamps. Driver time, publication cost and scheduling can affect results; measure physical output independently. No new claim of real-time headroom.

Weak serial-only UI accepts d for a one-line snapshot at 115200 baud. Strong visual hook supersedes this diagnostic, so the visual worker should optionally expose a snapshot through its own low-rate UI diagnostic, without using it to renew the heartbeat or blocking audio. Do not poll/print on audio task. For physical qualification after owner approval, reboot per configuration to reset lifetime maxima; compare sustained no-touch idle, continuous six-patch XY, and visual-heavy switching at each quality tier. Record duration, attempts, delta failures and budget counters, maxima, visual cadence, audible dropouts and external audio trace if available. Do not infer zero underruns merely from zero write failures. No physical flash/audition performed.

## Native receipt

```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pthread -Iinclude tests/timing_test.cpp -o /tmp/timing_test
/tmp/timing_test
clang++ -std=c++17 -O1 -g -Wall -Wextra -Werror -pthread -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tests/timing_test.cpp -o /tmp/timing_test_san
/tmp/timing_test_san
```

Both local commands passed 2026-11-26: threshold boundaries, wrap across 0xffffffff, failed/short-write representation, first-block gap exclusion, normal blocking write separation and 200000 concurrent publications. Optimized 8251 coherent reads; sanitized 60954 (scheduler-dependent). Tested blobs d61132f829ff6dc7d8d3c769e3518e6b5c375d57 and c0fcf6dbe6741cea3ff44881d4f938eaa86c27a2. Existing host/target workflow extended by one regression step; target compile pending. No local PlatformIO; git ls-remote DNS failure. No upstream code copied. Reinspected Diviner HAL and MIT license at 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC README 495a98ca12bae467a507031f04931aa4254b1866, FM-1-MHAC README 16c698d9e244559064a9a9a22f834d36912bf498, Golden Receipt engine eb78ea3b7a08a646910e66235724f259aeeb8190. No visual-owned file changed.
