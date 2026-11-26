# CYD-AUDIO-009 — Explicit sound-only NVS storage

Refs #1, cumulative Draft/HOLD #2. Parent 43eb4db64146b6bd902d64b7a9f4fb2bc62bb846. Prior audio run 45774081900 and integrated visual run 45776658123 reported success. No DSP, control, feature, timing, preset format, or visual-owned source changed.

One sound slot, ESP32 Arduino Preferences namespace mhac-audio, key sound-v1, exactly the existing 12-byte MHPS v1 record. No new dependency, no NVS erase, no automatic save/recall, no gate or mute in the record. The UI-only backend checks exact size, codec integrity, write return count and readback equality. An identical stored record avoids the write altogether. Failed or malformed recall preserves the selected preset; no claim of transactional rollback or physical power-loss survival. Read failures or wrong length report unavailable.

Audio-only weak serial UI: A–L and 1–8 explicitly audition; m mutes, then wait at least 250 ms before s (save selected) or r (recall selected without publishing any control). p explicitly auditions the current selection. Two-second audition auto-mutes; 0 releases but does not count as explicit mute. No auto-arm, renewal, boot recall or silent write during playing. Visual strong hooks remain unaffected; they may use the generic store contract later, but this change does not implement visual preset storage.

The quiet guard is UI intent and elapsed time, not proof of hardware silence. NVS operations on UI task can still stall flash access across cores; device deadline/underrun measurements and actual reset/flash persistence tests are required before enabling this in live performance. No I/O, allocations or locks added to DSP, no physical tests. Native fake backend tests model absent storage, reboot reconstruction, corruption, duplicate writes, denied writes, failed verification, invalid coordinates and rollover. They do not model ESP32 NVS internals.

Local GNU optimized and Clang ASan/UBSan PASS: 12 save/reboot/recall/duplicate/failure sets, 1152 corrupted records, 6012 quiet guard checks. Exact tested header/test blobs fdb23e2eb9b1ec03aa385aad535adeecce102e36 / 6c809de5105135901310ffacd443536e2300b4d7; predecessor codec d1b41d63b3a2b6872d81792f46d31347038dd3ce. Reproduce:
    g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/preset_store_test.cpp -o /tmp/cyd_store
    /tmp/cyd_store
    clang++ -std=c++17 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tests/preset_store_test.cpp -o /tmp/cyd_store_san
    /tmp/cyd_store_san

Existing narrow workflow extended once. Local DNS and PlatformIO unavailable; target compile and physical persistence not yet qualified. Read Diviner LICENSE and HAL at 82e06e11e5947e41e888716e7a68e99187410f2e, MHAC README at 495a98ca12bae467a507031f04931aa4254b1866, FM-1-MHAC README at 16c698d9e244559064a9a9a22f834d36912bf498 and Three Winters PR #597 at 91dc53aca2076665693cbef9f0c61fd913b28745. Original code, no upstream reuse, no license changes, no scientific findings.
