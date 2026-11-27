# CYD-AUDIO-035 — stable silent-block acceleration

Refs #1; cumulative Draft/HOLD PR #2. Exact parent 7e514c1410d359bfe6fd9f43b7739ef50a381bc1, audio run 45981558589 SUCCESS (artifact 15188898352). Integrated VISUAL-034 9863b2bd0273824f5728b74ea49e711774f53c41 run 45983264616 SUCCESS; Windows artifact 15190332192 and ESP32 artifact 15190357233 exact-head and unexpired by metadata. No independent archive download this run.

Optimize stable silent non-modal blocks without changing sound, control ABI or features. The fast path requires nonzero block size, no active samples, exactly zero global envelope, fully settled patch weights and a non-modal patch. It preserves per-sample pitch/morph smoothing, all phase increments, motif clock and rhythm envelope, feature windows, watchdog and packed analysis. Echo and unused voices are reset as in the original. Modal and transitional blocks keep the general path. Phase and motif helpers share the original arithmetic. No new state, allocation, dependencies or hardware changes. This is engineering optimization, not a research result.

Local source: prior AUDIO-034 audition ZIP verified all manifest entries; four DSP blobs and counter-colour test match pinned remote source. Compiled both exact parent and new header with:
```sh
g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tools/audio/idle_cost_test.cpp -o /tmp/idle
/tmp/idle /tmp/trace.bin
/tmp/idle --bench
clang++ -std=c++17 -O1 -g -ffp-contract=off -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -Wall -Wextra -Werror -Iinclude tools/audio/idle_cost_test.cpp -o /tmp/idle-san
/tmp/idle-san /tmp/san.bin
```
GCC parent/candidate and Clang candidate traces byte-identical (PCM, packed analysis, features, watchdog receipts), SHA256 3be83daf7a86f247cab8f220fbae45179426fe2cea4a411bfbe484530ce2854e. All 81 ordered sound pairs, 7,464,960 samples and 58,320 feature windows per execution, varying partitions, initial mute, sustained sound, idle patch changes, simulated producer stall and stale renewal, observed release/rearm, final mute. Bounds, phase partition, feature clock and one trip per scenario PASS. Existing counter-colour test GCC PASS 80 colour targets, 30 articulation boundaries and 2,592,000 partition comparisons. Engine 3444 B, Quotation 72 B unchanged on host. Tested blobs f756d8289867ad6eb62bad0a31bd065249e9ca23 and 52da8429035d48c6e9cdfe9d340dce683898bd21.

Host benchmark, last sequential parent/candidate GCC run: seven 10000-block batches per mode/condition after 150-block warmup; median batch average ns per 128 samples including control publication. Not a hard real-time latency test; machine noise and code layout affect results.
|Mode|Idle parent|Idle candidate|Active parent|Active candidate|
|---|---:|---:|---:|---:|
|Phase Morph|2274|710|2352|2344|
|Harmonic Lattice|2304|746|2494|2341|
|Ring Dialect|1868|714|1999|1941|
|Detuned Swarm|2094|711|2138|2079|
|Erasure Pulse|2909|774|3112|2955|
|Recursive Echo|1617|711|2062|1981|
|Modal Chime|1211|1256|1633|1599|
|Quotation|1499|714|2584|2408|
|Counterquotation|1490|738|2723|2523|

Modal deliberately not accelerated; small benchmark slowdown in idle modal case, active times varied across preliminary runs. No target timing claim. Existing job adds functional witness only, no extra jobs, no timing threshold. Target build pending. Local PlatformIO unavailable. Read-only references: Diviner README/MIT 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README 495a98ca12bae467a507031f04931aa4254b1866; FM-1 README 16c698d9e244559064a9a9a22f834d36912bf498; Trace Bell VST3 notes 408af1d6c2a3fdc2a444bb36f44ac0b2ce44be64. No upstream reuse. Next: exact-head CI, visual integration, owner physical measurement under concurrent display/touch and audio. No main merge or flash approval.
