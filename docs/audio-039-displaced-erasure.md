# CYD-AUDIO-039 — Displaced Erasure (Refs #1, #2, #4)

Base 1155e368b233438fb255b19a8059a24c94875da7; audio qualification 46015162660 PASS; integrated VISUAL-038 c810d4e1677e0ec30464a511d81d1edc6abfab40, run 46016793789 PASS. ErasurePulse retains the original second-onset suppression and adds a quiet displaced 550-sample fragment at motif phase 5100..5649 (212.5 ms from period start; 94.5 ms after the second onset). Target gain .45*Y; existing smoothing and oscillator reused, no new state. Y=0 remains exactly unchanged. This is creative musical exaptation of omission/quotation, not a formal MHAC implementation or perception claim.

Host commands from repo root:

    g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tools/audio/erasure_relocation_test.cpp -o /tmp/erasure
    /tmp/erasure /tmp/erasure-
    clang++ -std=c++17 -O1 -g -ffp-contract=off -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Werror -Iinclude tools/audio/erasure_relocation_test.cpp -o /tmp/erasure-san
    /tmp/erasure-san /tmp/erasure-san-

Repeat flags for tools/audio/echo_taps_test.cpp. Both compilers pass 720000 fixed-Y partition comparisons and 2880000 catalogue partition comparisons; 13/13 candidate PCM recordings cross-compiler identical. Six steady-state cycles: second onset full-Y energy remains 13389..13577 vs zero-Y 23679481800..23685153359 (same as base); displaced window at full Y 982394226..983414753 vs base zero, half-Y 367969671..368633702. Differential with exact base (headers checked against Git blob IDs): eight other modes and dry echo unchanged, 1296000 samples; zero-Y ErasurePulse unchanged, another 120000 samples. ErasurePulse variable-gesture recording changes 21631/144000 samples. Engine 3448 bytes, unchanged. Tested header d04514452833b0572e94d02d9d8fcd6fc5b42678 and test 951550344227c9982d5d25dc06ead4df3fdb6614. Existing workflow extended, no new job. Target qualification pending; DNS blocks local Git, no physical measurement or flash approval.

Read-only references rechecked: Diviner platformio.ini 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README 495a98ca12bae467a507031f04931aa4254b1866; FM-1-MHAC README 16c698d9e244559064a9a9a22f834d36912bf498; Palimpsest Shuttle CMakeLists.txt a832d89601154c404aa1570244d5d790ab8e1243. No upstream code copied. No patch/contract/visual changes. Next: exact target qualification, visual import, physical concurrent timing and audition.
