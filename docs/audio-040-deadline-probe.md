# AUDIO-040 — host audio deadline and mailbox-contention probe (Refs #1, #2, #4)

Audio base 83616abc38fe60cf1817023d4233a7aa9a411522; qualified integrated visual head d23b5bde4160c7bdb33faf31fe14dff02a4ecfbf, run 46035288092: Windows and ESP32 jobs success. No DSP, control, preset, platform, or workflow modifications. The new standalone benchmark exercises the exact existing engine, feature mailbox and DAC quantizer from AUDIO-039: verified Git blobs d04514452833b0572e94d02d9d8fcd6fc5b42678 (audio), 19cfc3bea8810998508d1dbef794118c08ca020a (features), 43a92b839515947a9843be9a131e1fbad6b85bd6 (modal), 70669144f56ba2b884bb2179471dd8292e1eaf0a (quotation), 8f7843cf9c129500a00be342b7b5e6679438f0e0 (DAC).

Run from repo root:

    g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -pthread -Iinclude tools/audio/deadline_probe.cpp -o /tmp/deadline
    /tmp/deadline > /tmp/deadline-gcc.csv
    clang++ -std=c++17 -O1 -g -ffp-contract=off -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Werror -pthread -Iinclude tools/audio/deadline_probe.cpp -o /tmp/deadline-san
    /tmp/deadline-san > /tmp/deadline-san.csv

Host compiler GCC 14.2 and Clang 17, virtualized AMD EPYC 9V74 x86_64. Each of 11 scenarios runs with and without a tight concurrent feature/analysis reader: nine modes (IDs 0–8), rapid patch switching (9), interrupted control lease (10). 64 warm-up + 4096 timed blocks per scenario/reader setting, 128 samples at 24 kHz. Timed region is render + dual-slot DAC preparation only, excludes input publication, PCM audit, DMA, scheduling model and physical SPI/display. This is an unpaced throughput/latency probe, not a real-time or actual visual load simulation. Output bounds, slot alignment, exact two lease trips and silence checked; per-scenario PCM/DAC hash identical with/without reader, and across both compilers. Timing is descriptive only, no flaky timing assertion.

GCC: 90112 timed blocks, 0 observed over nominal 5333.333 us block duration. Highest p99 23.646 us (rapid switching, no reader); highest p99 with reader 20.491 us (Modal Chime); max observed 732.612 us. Modal median 6.039 / 6.339 us, rapid switch median 4.407 / 4.677 us (without/with reader). Clang ASan/UBSan: same 90112 blocks, 0 observed over budget, 11/11 scenario hashes exact across all four compiler/reader configurations; max 801.555 us. Both compilers peak 8190 PCM units, no unexpected lease trip. No TSan or device timing claim; host timings are not a bound. Probe source Git blob 2dc8a663753c8c23baed1a83dcd93bb203bd58af.

Existing target task already measures render, prepare, write and start gap separately (src/main.cpp); use that instrumentation in owner physical run with full-screen modes, fast gestures, sustained notes and rapid preset changes. Physical audition, DMA underruns, ESP32 headroom and board validation remain HOLD. No remote flashing.

Read-only refs rechecked: Diviner platformio.ini 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README 495a98ca12bae467a507031f04931aa4254b1866; FM-1-MHAC README 16c698d9e244559064a9a9a22f834d36912bf498; Palimpsest Shuttle CMakeLists.txt a832d89601154c404aa1570244d5d790ab8e1243. No upstream code reused. Next: keep qualified VISUAL-040 download stable, then prioritize a meaningful new playable synthesis capability and physical timing receipt collection.
