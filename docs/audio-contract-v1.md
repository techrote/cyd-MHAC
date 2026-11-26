# CYD-AUDIO-001 — provisional v1 interface

Use this exact branch commit as a base/pin, not an unpinned moving head. Visual lane owns strong definitions of mhac_ui_begin and mhac_ui_tick in a separate source file; do not edit audio-owned src/main.cpp. Include mhac_platform.hpp. Call exchange.publish(x,y,gate,mute) from one UI producer only; the weak serial diagnostic is replaced by the strong UI hooks. Boot muted. X and Y unsigned 0..4095, X maps exponentially to 110..880 Hz, Y morphs sine to phase modulation. Gate is held touch; release on touch-up and on lost/stale gesture. Mute ramps down (not emergency hard cut). UI must explicitly unmute; use conservative output. No audio-thread waits on UI.

Control atomic word: X bits 0..11, Y 12..23, gate 24, mute 25. Analysis snapshot: peak bits 0..11 (twice value approximates signed PCM absolute peak), oscillator phase 12..23 (fraction of turn /4096), render sequence 24..31 modulo 256. Single atomic coherent word; no payload outside it. Sequence increments per render (5.333 ms for fixed device block), not wall-clock timestamp; UI may timestamp its own read and must handle wrap/missed blocks. audio_blocks counts successful DMA writes, audio_errors counts failed/partial writes; neither proves absence of underruns. No display or transport dependencies in render. Future version must explicitly negotiate changes.

## Reference and limits

Diviner reference commit 82e06e11e5947e41e888716e7a68e99187410f2e: platformio.ini and src/hal_cyd.cpp, MIT license at that commit. Observed LCD SPI2 SCLK14/MOSI13/MISO12/DC2/CS15, touch VSPI CLK25/MOSI32/MISO39/CS33, I2S0 DAC left GPIO26 only (GPIO25 would collide with touch). Platform and partition selection follow reference; actual partition binary and flash size unverified here. Original DSP/interface/adapter, no upstream file copied. MHAC main 495a98ca12bae467a507031f04931aa4254b1866 and FM-1-MHAC main 16c698d9e244559064a9a9a22f834d36912bf498 read-only. This tone is a creative foundation, not a semantic protocol implementation. No foundry code reused.

## Reproduce

g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/audio_test.cpp -o audio_test
./audio_test

Local GNU C++ optimized and ASan/UBSan executions PASS: 127971 nonzero samples; sample-word FNV recurrence 49d6545897c9e1c6 (toolchain-specific floating-point receipt, not cross-platform bit-exact promise). Tested header blob 14289b9f60cc4ff4d30507e3d5cdc26095e33e0b; test blob 93b1f635f2209cf008dee39a6c2283d503ec464c. 128000 active samples, identical independent engine instances, bounded amplitude, boot silence, release to exact zero. No concurrency stress or physical test. Local DNS blocked; PlatformIO absent.

Target build: pio run -e cyd. No flash instructions or approved firmware until exact target build, partition/offset review and recoverable backup procedure verified. Default serial diagnostic accepts 1/2/3 for two-second tones, 0 release, m mute; not yet run on hardware. Next: build target, integrate visual touch hook, measure DMA errors, jitter, actual underruns, CPU and memory under display load. No USB PCM yet.
