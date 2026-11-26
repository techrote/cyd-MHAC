# CYD-AUDIO-019 — full-bank gesture audition (Refs #1, #2, #4)

Parent 19b4153e21aa26ff5c339e7978b8e077fa67748b. Offline host fixture only, no engine, ABI, target driver, UI, storage or workflow changes. VISUAL-017 exact 83693411f0cc6d954ddb516c0f5b43a64a4e9789, run 45869178621 completed successfully; matched Windows/ESP32 artifacts 15141717007 and 15142021522, as recorded by visual lane. Physical qualification still HOLD.

## Reproduction from repository root
```sh
mkdir -p audition-output
g++ -std=c++17 -O2 -ffp-contract=off -Wall -Wextra -Werror -Iinclude tools/audio/gesture_audition.cpp -o gesture_audition
./gesture_audition audition-output > metrics.csv
clang++ -std=c++17 -O1 -g -ffp-contract=off -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tools/audio/gesture_audition.cpp -o gesture_audition_san
./gesture_audition_san > metrics-san.csv
diff -u metrics.csv metrics-san.csv
```
No arguments runs regression without writing WAVs; optional argument is an existing directory. 16 factory sounds each receive a fixed six-second gesture score at 24 kHz: three strikes, pitch/timbre sweep, release, another held contact with explicit mute. Coordinates saturate at the existing bounds; not a touch-driver or lattice replay. Compare exact PCM at 128-sample publication boundaries with internal chunks 128, 31, 97. Check boot and final mute silence, live signal, bounded peak and no watchdog trip. Export reference PCM and both digital DAC approximations at original gain. The WAVs encode ideal quantizer output, not the ESP32 analog transfer, speaker response or subjective sound. No normalization.

## Executed local evidence
GCC 14.2.0 optimized and Clang 17.0.0 ASan/UBSan both PASS; identical 16-row metrics and all 48 sample-word hashes across the two executions. 2,304,000 reference samples per compiler, 4,608,000 exact sample comparisons against two partition variants, 48 WAV files independently checked for mono 16-bit/24 kHz/144000 frames and exact initial/final silence. Negative control peak bound changed to 100 rejected on first preset. Peak 3206–8190 PCM units. Whole-file quantizer error mean range floor -86.405 to -55.485; feedback -0.0101 to +0.0159 PCM units. Full-band RMS error floor 101.39–123.93; feedback 62.11–83.09 PCM units (includes silence; not perceptual or in-band weighted). These local hashes are toolchain receipts not cross-platform golden constants. Tested tool blob b146ce07b4abbb6f30bc1c82849c2f397abca495; all seven dependent audio header blobs verified against the exact parent, recovered from AUDIO-017 archive. Local Git DNS unavailable, PlatformIO absent; no fresh target or Windows execution, no need to rebuild offline-only utility to preserve previous frozen candidate.

Read-only reference review: Diviner 82e06e11e5947e41e888716e7a68e99187410f2e HAL and MIT LICENSE; MHAC 495a98ca12bae467a507031f04931aa4254b1866 README; FM-1-MHAC 16c698d9e244559064a9a9a22f834d36912bf498 README; acceleration c6392571e15088059594ddb87150f9495dcbac32 upgraded_chain_host.py. No upstream code copied. Sound concepts are creative exaptations, not a validated communication protocol. Next: actual owner audition and physical legacy/feedback DAC comparison, preserving stock recovery and hardware gates. Draft/HOLD; no flash or merge approval.
