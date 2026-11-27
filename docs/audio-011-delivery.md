# CYD-AUDIO-011 — independent target image integrity and recovery evidence

Refs #1, #2, #4. Parent 8c05f7d39a50aa61a0cf5ea8dddef44852446dbf; visual exact candidate 43552061b29b0bb43bf50a1515026c77e498d93a. Both exact-head workflows completed successfully on 2026-11-26. Downloaded original archives and compared their SHA-256 with GitHub artifact metadata. Both source receipts, firmware manifest hashes, ESP32 chip ID 0, declared 4 MiB flash, five segment extents, XOR and embedded SHA-256 passed independent Python parsing. No hardware is connected and this is not a signed or trusted-boot assertion.

| Candidate | Run / artifact | Archive SHA-256 | App SHA-256 |
|---|---|---|---|
| AUDIO-010 | 45790747347 / 15101899228 | e74a2e3f23f0d1f97b07d4ed6509c86dd2d938e1dd9263bf543e78e6e95438e5 | 03bdad543d49ff357110e39ff777738dc9775e73360da3eb44671cb21cc41001 |
| VISUAL-010 | 45794031808 / 15104526922 | ef1ca951126cc521308c200749b5084946c1b182f2331947902ace62f88c1b79 | 44481c7ca0f6f8866243099632d5ec165951a24c206671b91afbcebcf738511b |

300448 / 385120-byte app files respectively. Build logs show RAM 24940 / 39188 bytes; application flash used 300085 / 384761 bytes against 3145728. Not dynamic memory peaks. All 24 actual-image corruption controls rejected. Synthetic partition tests: 2 positive shapes and 9 invalid layouts/corruptions rejected, including valid-MD5 overlap and out-of-bounds. No claim the previous artifacts contain a partition binary: they do not.

## Offline verifier

Python 3.9+ standard library. No serial access, flash writes or dependency install. Example:
```
python tools/audio/verify_candidate.py github-actions-artifact-15104526922.zip --source 43552061b29b0bb43bf50a1515026c77e498d93a --archive-sha256 ef1ca951126cc521308c200749b5084946c1b182f2331947902ace62f88c1b79 --negative-tests
python tools/audio/test_verify_candidate.py -v
```
Tested local tool blobs 54728922f471ee41e0852d70f560653266e5d81e / 6dd6c12470ca2a41eaa6cf9ce41ac07ac8e10d95. Strict unsigned hashed app profile only; no validation of signatures, authenticity, eFuses or physical flash geometry. Firmware format reference: https://docs.espressif.com/projects/esptool/en/release-v4/esp32/advanced-topics/firmware-image-format.html . Partition format reference: https://docs.espressif.com/projects/esp-idf/en/v5.0.2/esp32/api-guides/partition-tables.html . Expected huge_app.csv read from arduino-esp32 2.0.17, blob 61254fcab25e721764b7a4e538b16c89ce4a066f; actual generated binary must pass CI separately. Existing Diviner 82e06e11e5947e41e888716e7a68e99187410f2e HAL and MIT notice, MHAC 495a98ca12bae467a507031f04931aa4254b1866 README, FM-1 16c698d9e244559064a9a9a22f834d36912bf498 README reread. No upstream source copied.

## Candidate and stock preservation gates

New narrow existing CI job captures generated partition and bootloader as evidence alongside the application, checks actual partition MD5, alignment, bounds and expected app slot, emits JSON and hashes all three. This is not yet a complete flash kit: boot_app0/init data, actual uploader offset configuration and owner board identification remain gates. No app or layout changes; no visual lane edits. Do not write an application-only image to address zero or assume the stock partition table matches. No erase_flash or write_flash instructions are authorized here.

For owner preparation only: verify PCB is the classic ESP32-2432S028R, check USB serial device/port and actual flash identification using esptool 4.11.0 read-only flash_id; stop if not 4 MiB ESP32 or if security configuration is uncertain. Before any later owner-authorized write, make two independent full 0x400000-byte read_flash backups from address 0, in download mode; compare length and SHA-256 (Windows Get-FileHash -Algorithm SHA256), save separately offline, never upload stock dumps containing credentials. If reads differ or read protection/secure boot/encryption is present, stop. A matching pair is not proof of restorability; review recovery method before any write. The official tool may reset the board when entering/exiting its loader. No physical device interaction in this run.

Next: exact-head CI and actual partition receipt, then coordinate complete integrated owner kit and hardware acceptance with visual lane. HOLD.
