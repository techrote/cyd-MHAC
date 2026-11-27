# CYD-AUDIO-031 — terminal startup fault propagation

Refs #1, cumulative Draft/HOLD #2. Parent 1297d4584cf1927363fb813f5ae0948841d0b327 qualified by run 45945348577. Integrated VISUAL-029 2d47432011dba01f6b3c67a89f9652ebb2c36d36 run 45946441585 completed SUCCESS. Windows artifact 15175166477 and target qualification artifact 15175022153 unexpired per Actions metadata; not downloaded or independently hashed this run.

The former audio_begin returned false on I2S install, DAC enable or task creation failure, but only printed a startup message. The strong visual UI already checks audio_errors each tick and forces mute on any nonzero count; startup failure was not published to that existing fault channel. This could leave a visually active but silent instrument. The new setup-only helper preserves the original driver cleanup order, publishes one terminal error on failure before returning, and leaves successful startup unchanged. No changes to pins, DMA configuration, DSP or audio-task code. The weak serial UI now latches any terminal error, publishes mute once and refuses further performance commands until reboot. The strong visual hook is unchanged and already consumes this counter. Its UI fault presentation is not tested on hardware.

The existing audio_errors counter now denotes terminal audio startup OR partial/failed DMA write, not just writes; audio_timing.failed_writes remains write-specific. No atomic type or contract layout change. A nonzero counter is still a reboot-required fault. No repeated retries or automatic recovery.

Executed local tests (mock setup operations, not SDK/hardware): GCC optimized C++17 and Clang ASan/UBSan each PASS install/DAC/task failure and success, exact cleanup call order and terminal fault count. Tested Git blobs: header 9e86a6ec3612dbde4f92592e907a9f3bb651c450, test cfad3ada83bfc2f361a41f284d71eb2017b49166.
```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tools/audio/startup_fault_test.cpp -o /tmp/startup-gcc
/tmp/startup-gcc
clang++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Werror -Iinclude tools/audio/startup_fault_test.cpp -o /tmp/startup-san
/tmp/startup-san
```
Existing audio workflow extended, no extra jobs. SDK target compile pending; local PlatformIO unavailable and GitHub DNS fails. Physical startup and write failure injection, speaker audition, actual underruns and concurrent display timing remain unqualified. Read-only Diviner HAL/MIT at 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC README at 495a98ca12bae467a507031f04931aa4254b1866, FM-1 README at 16c698d9e244559064a9a9a22f834d36912bf498, Trace Bell core at d2688449e53dec440f690de5270b23b9c49e1e6d; no upstream source reused. No main merge or flash approval.
