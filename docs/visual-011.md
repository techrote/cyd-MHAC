# CYD-VISUAL-011 — portable Windows audition qualification (Refs #3/#4)

Previous exact visual head 43552061b29b0bb43bf50a1515026c77e498d93a, run 45794031808 SUCCESS; independently executed its archived Linux binary using local SDL2 dummy audio/video: 147 callbacks, 91 active, 46 frames, loop/capture PASS, final peak 0. Command: `SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./mhac-desktop-linux --smoke local-smoke.ppm`. No speakers or device tested. Imports exact AUDIO-011 6cc2810c4eace5d4960facb48be64569c5d1eddb, run 45800105732 SUCCESS, four disjoint delivery files unchanged. No DSP, hardware or interface changes.

Added one native Windows x64 job to the existing narrow visual workflow (public repo, standard Windows 2022 runner, 20-minute cap). Build SDL2 from official release-2.32.10 commit 5d249570393f7a37e037abf22cd6012a4cc56a71; checked upstream CMake options and LICENSE.txt at the exact pin. Static SDL2, MSVC static CRT, no SDL test targets or HIDAPI. No paid runner or device needed. CMake checks the checkout commit. Windows Release tests explicitly undefine NDEBUG; the build uses /fp:strict. The job runs the actual callback/gesture smoke, stages the executable with license notices and launcher in a path with spaces, restricts PATH to Windows system directories, reruns the smoke there, and only then produces the ZIP and hashes. The package remains unqualified until actual CI completes. No external binary obtained or executed locally for Windows; local DNS, SDL development headers, Windows compiler and Wine are unavailable.

New regression extends desktop audio test from six to all seven sound modes using the actual v3 control publication helper. Locally executed optimized GCC and Clang ASan/UBSan PASS: 1,400 variable/unaligned callbacks, exact PCM equality with reference Engine, bounded peak, release and independent watchdog silence. Commands:
    g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tests/desktop_audio_test.cpp -o desktop-test && ./desktop-test
    clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude tests/desktop_audio_test.cpp -o desktop-san && ./desktop-san
Test blob 87f3ca43e52ef3266dca74ea7eed0133ce6cdb68; exact input blobs a131b86c03ad7c5e813551ee331a940bfa78634e (audio), 8a6b02c1614caa20431664c81bb47f43a0a9a93d (desktop adapter), 19cfc3bea8810998508d1dbef794118c08ca020a (features), ef110b7194cccc94fb8635acb0d9008ff823f229 (modal), f9b63d83fe1401565cfa7186145a5e2415f8e691 (patch).

Build locally on Windows with Visual Studio 2022 C++ tools, Git and CMake >=3.24:
    git clone https://github.com/libsdl-org/SDL.git _sdl2
    git -C _sdl2 checkout --detach 5d249570393f7a37e037abf22cd6012a4cc56a71
    cmake -S tools/desktop -B build-win -G "Visual Studio 17 2022" -A x64 "-DMHAC_SDL_SOURCE=$PWD/_sdl2" -DCMAKE_POLICY_VERSION_MINIMUM=3.10
    cmake --build build-win --config Release --parallel 4
    ctest --test-dir build-win -C Release -V
Use hosted artifact visual-windows-audition after SUCCESS; unzip inner candidate ZIP, launch Open CYD-MHAC.cmd. Owner interactive audition and hardware deadline/calibration gates separate. Existing Diviner HAL 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC README 495a98ca12bae467a507031f04931aa4254b1866 and FM-1 README 16c698d9e244559064a9a9a22f834d36912bf498 rechecked. No new research code reused. No main merge or flash authorization. Next: recover exact Windows build receipts and repair any native failures, then calibrate physical touch.
