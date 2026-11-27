# AUDIO-041 — Formant Reed, a session-only v6 extension (refs #1, #2, #4)

Base 3b519c11b3998f0c9eabb10eea505c4a6c69f0e2, AUDIO-040 host/target workflow 46043054150 success. Integrated visual d23b5bde4160c7bdb33faf31fe14dff02a4ecfbf, both jobs success 46035288092. No evidence of an unfinished preceding branch update: both exact heads match the last checkpoints and both CI runs completed. Keep these qualified packages; this is a new audio candidate, not integrated or physically approved.

## Sound and contract

Formant Reed is an original eight-partial additive vowel/reed-inspired sound: X is the existing 110–880 Hz glide, Y moves the two triangular spectral windows from 350/2300 to 1000/1300 Hz. Fundamental always retained; sum is normalized with a 0.20 denominator floor and the existing smoothed envelope/8191 gain. Uses existing phase and approximate sine, no physical vocal tract, speech decoder or research protocol claim. Nominal eighth partial <=7040 Hz; the approximate sine is not mathematically band-limited. No new oscillator, allocation, buffer, lock or I/O in render. One additional four-byte mixer weight, preserving all prior weights and transitions when not selected.

Opt-in: session ID 9, formant_extension_version=6, Exchange::publish_formant(x,y,gate,mute). Wire ID 1 plus all three high extension flags bits 29/30/31; partial flags remain legacy Harmonic Lattice, older engine falls back to Harmonic Lattice. Existing IDs, mailbox, persisted codec, presets and feature layout unchanged. Audio-owned weak serial diagnostic V selects a two-second example (220 Hz, Y=2200); p replays, 0/m release/mute. Strong visual UI overrides this diagnostic; integrated UI must explicitly expose session ID 9 and invoke publish_formant. Do not save session ID 9 in old codec. Next integration: add a paired visual preset/selection, preserve original nine engines and all qualified artwork, qualify exact combined source. No flash approval.

## Reproduce

    g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tools/audio/formant_test.cpp -o formant_test
    ./formant_test
    clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tools/audio/formant_test.cpp -o formant_san
    ./formant_san
    g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -pthread -Iinclude tools/audio/deadline_probe.cpp -o deadline_probe
    ./deadline_probe

Local GCC 14.2 and Clang 17 ASan/UBSan PASS; 36 six-second scenarios, 5,184,000 exact partition comparisons per compiler, 36/36 identical sample-word hashes across compilers. 30 legacy scenarios (including rapid nine-mode switching) match the exact AUDIO-040 base hashes using the same test with -DLEGACY_ONLY and predecessor headers; six candidate WAVs byte-identical across compilers. All 7 partial/no opt-in flags preserve legacy ID 1, 75,076 legal grid gain checks, one expected watchdog trip per scenario, stale rearm refusal and final explicit mute silence. Engine size 3452 B versus 3448 B. Initial test incorrectly demanded ordinary Modal Chime release be silent after <1 second; base and candidate both refused. Test corrected to use explicit final mute while preserving the separate watchdog checks; no DSP change for the negative result. All three tested source blobs: 3a48b29a02218a3b39e53fb2dc19496b71f08e04, 51bec64a9f2b71b7476a12e1b7fbb2db6350011d, 47631aa7a5cb47592fd26d64ce6c52fdf19a1402.

Native GCC deadline probe extends AUDIO-040 to ten modes, 12 scenarios x two reader configurations, 98,304 timed blocks. Formant Reed median 6.961 us without reader / 7.091 us with reader, p99 10.455/19.029 us, max 209.054/480.313 us, zero observed formant blocks over nominal 5333 us. Hash 26990ba8ae5676b7 both configurations. No ESP32 timing, SPI/DMA, display contention, physical audition or subjective sound claim. Local PlatformIO absent; existing workflow adds native test without new job; target compilation pending.

Read-only upstream refs checked: Diviner hal and MIT LICENSE 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README 495a98ca12bae467a507031f04931aa4254b1866; FM-1-MHAC README 16c698d9e244559064a9a9a22f834d36912bf498; Foundry-adjacent Palimpsest Shuttle CMakeLists a832d89601154c404aa1570244d5d790ab8e1243. No upstream code copied. Creative exaptation only.
