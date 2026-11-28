# VISUAL-036 — crossed caustic light sheets (Refs #3)

Draft/HOLD. Visual parent 185330fd2e45f24a98776f7790572c4e8dffa3f5, run 45991844935 SUCCESS. Audio parent 92322f14d54165b3c146034141fbe5c733074f4b, run 45997785268 SUCCESS: import three audio-owned blobs exactly. Previous Windows artifact 15193613985 and ESP32 artifact 15194710443 unexpired and source-matched by metadata; not independently downloaded.

Enhance existing Caustic mode (8), preserving mode count, palette transform, gesture overlay and safety controls. The original two fixed curved sheets now cross a third gesture-centred parabolic filament. Intersections receive a contrasting warm flare controlled by Y, with aggregate peak modulating the underlying lens. No new persistent state, allocation, floating point, audio coupling, pins, or interface changes. Integer artistic field only; not optical ray tracing or an acoustic measurement. Actual device rendering cost not measured. Existing integrated caustic study retained (actual DSP, three quality tiers, safety); add standalone shade witness to same Linux job.

Executed local GCC optimized and Clang ASan/UBSan C++17:
    g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/caustic_layers_test.cpp -o test && ./test frame-
    clang++ -std=c++17 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude tests/caustic_layers_test.cpp -o test-san && ./test-san
Both PASS 460800 deterministic bounded RGB565 pixels, six distinct frames, matching hashes across compilers. Synthetic control profiles, not device or live audio captures. Tested blobs 2eae878d3cab803fcef75e9a236c7d6e5ca2852c and b5ea407a1e2079536858a2808e1787c4196916ca. Integrated renderer and target/Windows CI remain outstanding. No new CI jobs.

Read-only references: Diviner theme, CYD/SDL HAL and MIT notice at 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README 495a98ca12bae467a507031f04931aa4254b1866; FM-1 README 16c698d9e244559064a9a9a22f834d36912bf498; Trace Bell VST3 docs d996a0b8c457630e3d18a4b2a06393640f8d3fbf. No upstream code copied. Creative exaptation, not perception evidence. No main merge or flash approval.
