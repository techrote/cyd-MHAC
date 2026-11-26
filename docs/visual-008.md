# CYD-VISUAL-008 — Interactive desktop audition candidate

Refs #3/#4. Exact visual predecessor d3333999b5ea24b095b0876dbd480ca6afb51a4d, hosted run 45768805764 SUCCESS (native + ESP32 build). AUDIO-008 exact input 43eb4db64146b6bd902d64b7a9f4fb2bc62bb846, run 45774081900 SUCCESS. Import its five changed blobs verbatim and retain both ancestry pins; no shared code edit. Factory preset selection not yet exposed in UI. Prior Diviner HAL pin 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC README 495a98ca12bae467a507031f04931aa4254b1866 and FM-1-MHAC README 16c698d9e244559064a9a9a22f834d36912bf498 re-read; existing Diviner MIT notice retained. New desktop adapter is original code, uses SDL2 through its public API, no upstream code copied. SDL2 is a desktop-only dependency (zlib license); system SDL2 version is recorded by CI, not an immutable binary dependency pin; no change to target dependencies.

## Build and use (candidate)
Linux with g++ and SDL2 development files:
    g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tools/desktop/main.cpp $(pkg-config --cflags --libs sdl2) -o mhac-desktop
    ./mhac-desktop

Windows: use an MSYS2 UCRT64 terminal with mingw-w64-ucrt-x86_64-gcc, mingw-w64-ucrt-x86_64-SDL2 and mingw-w64-ucrt-x86_64-pkgconf installed. Same build command with -o mhac-desktop.exe and ./mhac-desktop.exe. Run within that terminal so the SDL2 runtime DLL is on PATH. No native Windows build or portable ZIP claimed yet. No network at runtime.

The actual 320x240 RGB565 renderer is displayed at 2x scale; left mouse button substitutes for a single touch contact. Boot muted. Top-right unmute, top-left visual, bottom-left sound and bottom-right scale; top-centre loop as in visual-007. Escape mutes and cancels loops, Q quits. Losing focus/minimizing cancels; leaving during an uncaptured mouse drag cancels. No physical touch, DAC or display simulation. No audio input or MIDI.

SDL audio callback exclusively owns Engine; UI publishes existing control and reads read-only analysis. Callback output is native 24kHz mono signed 16-bit, requests 256 samples, splits arbitrary callback size into <=128 sample chunks using fixed stack buffer. No heap, lock, logging, UI or clock calls in callback. UI stalls trigger existing sample-count watchdog (not a physical deadline measurement). Software renderer uses original full-quality strips, copies them to a desktop-only RGB565 frame buffer and presents at nominal 30fps independently of audio. No target allocation changes. Actual host audio device buffering/latency depends on driver; no physical audition claim. SDL dummy smoke does not play to speakers.

## Executed local qualification
Audio headers recovered from AUDIO-008 audition kit; Git blob hashes matched recorded exact commit blobs: engine 7e163a1c04cdbd520336e13fe66239bde472f763, features 19cfc3bea8810998508d1dbef794118c08ca020a. Local optimized GNU and Clang ASan/UBSan both PASS:
    g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tests/desktop_audio_test.cpp -o /tmp/desktop_audio_test && /tmp/desktop_audio_test
    clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tests/desktop_audio_test.cpp -o /tmp/desktop_audio_san && /tmp/desktop_audio_san
1,200 varying-size and intentionally unaligned callback buffers, odd trailing byte, guard bytes, all six patches, exact PCM vs reference chunked Engine, peak bounds, release silence and independent watchdog silence with stalled UI. Tested header blob 8a6b02c1614caa20431664c81bb47f43a0a9a93d and test f2668581410637f1d9655d665e30523a27f176a2.

Local Git DNS still blocked; SDL2 runtime present but headers absent; full desktop app not locally compiled. Existing single visual CI job now builds desktop and runs dummy-driver smoke (real Engine callbacks + UI loop/gesture + actual RGB565 render, focus-loss mute and silence, PPM capture) before target build. Require exact-head result, interactive owner audition and physical evaluation separately. No claim of Windows runtime, SDL smoke success, target build or flashing approval for this revision until evidence exists. Next: inspect smoke evidence, provide Windows portable bundle with verified SDL runtime/license, add bank selection and calibration.
