# CYD-AUDIO-017 — bounded 8-bit DAC error feedback

Refs #1, cumulative Draft/HOLD #2. Parent 40f4ebcc3ffd4ad85a1b8b67d9f5bdefd1c7f86b. Prior AUDIO-016 run 45848116845 and VISUAL-015 exact cd8aa0dd7d00acf72679dae7423d79cdd198f865 run 45851309299 both SUCCESS. The visual run's Windows and target jobs both passed; artifacts 15133712510 and 15134337380 belong to that exact visual head. The existing frozen VISUAL-014 owner candidate is unchanged.

The original DAC packing floors the signed 16-bit engine PCM to the high byte, making negative sub-LSB samples a negative 1-LSB output while positive sub-LSB samples become zero. This can create asymmetric quiet tails and negative quantization bias. New candidate uses first-order integer error feedback at the hardware packing boundary, retains state across DMA blocks and resets residual to zero for exact zero PCM. Output code is saturated to 0..255; residual is clamped to -128..127. No heap, locks, division, I/O or DSP changes. Both I2S slots still receive the same word; GPIO selection, sample rate, timing mailbox, patch and control protocols unchanged. Build with -DMHAC_DAC_LEGACY_FLOOR added to existing build flags to restore original packing. Not a DAC resolution or analogue fidelity claim; high-frequency noise/idle patterns and amplifier response require actual audition. The desktop engine still renders 16-bit PCM.

Native regression executed 2026-11-26 with GCC optimized and Clang ASan/UBSan, both PASS, identical text results. 16,777,216 state/sample transitions (all 256 residuals x all int16 samples), 131,072 constant +/-1 samples, 1,536,000 actual engine samples across all 16 factory presets, alternate packing partitions and exact zero/reset, final silence, saturating extremes. All preset cumulative signed quantization errors improved versus floor. Four-second score per preset: mean floor error range -51.1165 to -10.5667 PCM units; error feedback absolute mean <=0.01011 PCM units. These are digital comparisons, not physical output measurements.

```sh
g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tests/dac_test.cpp -o /tmp/dac_test
/tmp/dac_test
clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tests/dac_test.cpp -o /tmp/dac_test_san
/tmp/dac_test_san
```

Local tested blobs mhac_dac.hpp 8f7843cf9c129500a00be342b7b5e6679438f0e0 and dac_test.cpp 3e6dc5bdc420943fb4a65909d785276e1d524d2b matched created Git blobs. DSP source from archived AUDIO-014 kit matched exact parent engine 5dff26749570eb5e65a7f85db2b1dd49d96e2225 and modal 3a8dd866a2aa158baa7d0bc944bba8359b951f9c. Local no PlatformIO, git DNS unavailable. Existing audio CI job extended, target compile pending. Diviner 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC 495a98ca12bae467a507031f04931aa4254b1866 and FM-1 16c698d9e244559064a9a9a22f834d36912bf498 README reread; no upstream code reused. AGENTS.md/RAG.md absent on main and audio parent.

Owner acceptance: retain previous frozen kit; compare legacy and new packing on actual speaker only after board identity, recovery and owner-authorized physical gates. Include quiet modal release, sustained low note and full-screen visual stress; note hiss/idle tones, audible interruptions, timing maxima, and captured analogue output if available. No extra runner, no physical write, no main merge.
