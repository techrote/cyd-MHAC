# CYD-AUDIO-033 — Counterquotation spectral call and response

Refs #1; cumulative Draft/HOLD PR #2. Exact parent 8d9ce19a127d6b9ff891884dbe7a81f8184b0d5e (run 45958270438 SUCCESS). Integrated VISUAL-031 9f8b69d1670c1ab2ccf6a123f7ffa4295a9e813d run 45959945177 SUCCESS; Windows artifact 15181005622 and target artifact 15180836439 nonexpired, source matched by metadata. No independent artifact download in this increment.

Counterquotation now alternates dark lower-octave calls and brighter upper-fifth answers. On recall only, even history marks use one quarter of captured timbre, odd marks use 0.5 + 0.5 times captured timbre. Existing oscillator, pitch transposition, blend depth, smoothing, lease, gate, control/feature wire and bank remain unchanged. Raw marks are retained: toggling back to quotation recalls untransformed history. No additional per-sample operations, oscillator voices, state or allocation. Creative MHAC exaptation, not a research protocol or perception claim. Sonic change intentionally confined to session mode 8; existing 0–7 preserved in the tested performances.

Executed local GCC and Clang ASan/UBSan:
```sh
g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tools/audio/counter_colour_test.cpp -o /tmp/colour
/tmp/colour
/tmp/colour --render /tmp/new-
clang++ -std=c++17 -O1 -g -ffp-contract=off -fsanitize=address,undefined -fno-omit-frame-pointer -Wall -Wextra -Werror -Iinclude tools/audio/counter_colour_test.cpp -o /tmp/colour-san
/tmp/colour-san
/tmp/colour-san --render /tmp/san-
```
Repeat GCC with parent headers and --render /tmp/old- (analytical new-mode assertions not applicable to old build). Parent audio/quotation/modal/features blobs from local evidence ZIP matched pinned remote source before testing. Both candidate executables pass 80 synthetic isolated harmonic target comparisons, 2,592,000 exact partition comparisons (128 vs 31/97; nine six-second performances), output bounds, initial/final mute, no unexpected watchdog; identical PCM for all nine modes across GCC/sanitizer. Eight unchanged modes: 1,152,000 predecessor/candidate samples identical. Mode 8 changes 88,633 of 144,000 samples, first difference at sample 14051 (after 12000-sample history plus initial mute); peak 8190 in both. Native sizes Quotation 72, Engine 3444 unchanged. PCM SHA256 old 4aaee20b526c074a9795f799ea988ebe0d7a1b49ae4f3352a36abaf360613b30, new 944b130d81d8cc79c760a7f34ddb3fe188cd52a23d766029ff6b76560b2a5966. These are local exact receipts, not platform-independent goldens. WAV audition generated from actual mono native DSP PCM at 24 kHz with no normalization, not DAC/speaker or subjective listening. Tested new blob IDs c5efe5adbf1e1ad2c7f9b0a6470cbeeb4d4fd23a (header), 70bdc38329a70cac03cf9ff0341324325e071d1a (test). Existing audio job extended; no extra jobs. Target compile pending.

Read-only Diviner README/MIT 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC README 495a98ca12bae467a507031f04931aa4254b1866, FM-1 README 16c698d9e244559064a9a9a22f834d36912bf498, Trace Bell pole and VST3 documentation 8079d09d9c0652464dce78ce8b96446a2e1abe50. No upstream source reused. No main merge, flash or hardware approval. Next: integrate exact qualified audio head, physical audition and measured concurrent load before larger per-sample DSP expansion.
