# AUDIO-036 — digital DAC spectral audit (Refs #1)

Parent 3e179d4fc8913b3846c12549a60b390141d7e6e4, AUDIO-035 run 45990347215 SUCCESS. Integrated VISUAL-035 185330fd2e45f24a98776f7790572c4e8dffa3f5 run 45991844935 both jobs SUCCESS, Windows 15193613985 and ESP32 evidence 15194710443 nonexpired exact-head metadata; archives not independently downloaded. No changes to runtime DSP, DAC algorithm, shared contract, visual files, hardware, or preset format.

New host-only tool uses actual mhac_dac.hpp (blob 8f7843cf9c129500a00be342b7b5e6679438f0e0) to compare legacy floor and current error feedback. Six coherent 16384-sample stimuli, 24 kHz, three signed constant controls and three sinusoidal stimuli. FFT of output minus input, one-sided energy including DC and Nyquist, Parseval cross-check. Low-band metric includes DC through 3 kHz, not a hearing model. Largest non-DC bin may be arbitrary among near-tied comb lines. No window needed for these periodic fixtures. Checks residual conservation between zeros, zero reset, bounded error, coherent DC bias, idle comb and tone low-band regression. 98,304 samples per packer per run. No device I/O.

Commands from repository root:
```sh
g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tools/audio/dac_spectrum.cpp -o /tmp/dac_spectrum
/tmp/dac_spectrum audition
clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -fno-sanitize-recover=all -Iinclude tools/audio/dac_spectrum.cpp -o /tmp/dac_spectrum_san
/tmp/dac_spectrum_san audition-san
```
Local GCC 14.2 and Clang 17 ASan/UBSan both PASS; CSV and three WAVs byte-identical between compilers. WAV format verified: 67152 frames, mono PCM16, 24 kHz, three AC tone segments separated by 6000 zeros, no DC stimuli in audition, no normalization. Tested source blob 63558445758a5dc0ddadd9b7edbfc0ec45bcdd6c. Existing audio job extended only.

|Stimulus|Floor DC-to-3 kHz RMS error|Feedback RMS error|
|---|---:|---:|
|+1 constant|1|8|
|-1 constant|255|8|
|+64 constant|64|0|
|375 Hz peak 64|150.391852|13.431145|
|750 Hz peak 512|123.398017|10.751333|
|1500 Hz peak 4096|96.013430|5.902388|

All feedback DC means zero in these fixtures. Important counterexample: +1 PCM constant becomes a 256-sample periodic pulse train (93.75 Hz repetition, tied harmonic lines) instead of constant zero output; total error RMS rises from 1 to 15.968719. This is a digital idle-pattern warning, not evidence of audible tones on the CYD. Do not change shipping quantizer or enable additional dither before physical A/B measurements. The existing legacy-floor build flag remains available. WAV feedback/floor/reference SHA256 respectively 7bf9f6c4a2d07c459b4e767f5d59c9fb8abac46876fe6989648d9d14bac5a7d1, 5403e23957683de1f5f9c1b45974ea38091eb722ac384b36e8250bf37f6586fb, 95f079e0f46dc9f925f9d7def0a4f46249f92286066545b84d101f4fd4240c4e. These are local receipts, not cross-platform goldens.

Read-only references: Diviner MIT and serial PCM header 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC README 495a98ca12bae467a507031f04931aa4254b1866, FM-1 README 16c698d9e244559064a9a9a22f834d36912bf498, Trace Bell notes d996a0b8c457630e3d18a4b2a06393640f8d3fbf. No upstream source reused. Local PlatformIO absent; new exact-head CI pending. Physical CYD measurements, actual speaker audition, DAC/amp transfer and deadlines remain unverified. Draft/HOLD; no main merge or flash approval.
