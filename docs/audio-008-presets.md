# CYD-AUDIO-008 — Factory sound bank and portable preset records

Refs #1, cumulative Draft/HOLD #2. Exact parent 25cd21b4d43f8f58db823a49c6e4faf04a047683. Prior audio run 45768143939 and integrated visual run 45768805764 confirmed successful 2026-11-26. No visual-owned file changed. DSP/control/analysis/timing headers are untouched.

## Format v1

Fixed 12 bytes, not a raw C++ object: ASCII MHPS (0–3), version 1 (4), patch ID 0–5 (5), little-endian unsigned 12-bit X (6–7) and Y (8–9), CRC-16/CCITT-FALSE (poly 0x1021, init 0xffff, no reflection, xorout 0) over bytes 0–9 stored little endian (10–11). Golden vector for patch 0 / X=0 / Y=0: 4d485053010000000000efd1. Invalid or unknown versions, reserved patch IDs, out-of-range coordinates or bad CRC reject without mutating destination. CRC is accidental-corruption detection, not authentication. Callers must supply exactly 12 bytes; no partial reads accepted by the typed API. Frozen v1 catalog has six patches even if later catalog expands. No gate, mute, heartbeat or gesture state is serialized. Loading settings must never automatically arm, renew or unmute audio. No NVS/flash storage yet: this is the byte-level persistence contract only.

Factory bank (A–L on audio-only weak serial interface): Soft Current, Glass Fold, Low Organ, Harmonic Lantern, Copper Bell, Ring Dust, Slow Swarm, Double Orbit, Four Marks, Missing Mark, Near Reflection, Recursive Wire. Two contrasting XY configurations per patch. The uppercase audition keys explicitly request a two-second tone; previous numeric keys 1–8, 0, m, d retain their mapping. The UI worker can adopt the sound bank independently; no change to visual pairing or touch behavior. These are designed settings, not yet listened to or rendered as a bank.

## Qualification

Native codec test executed locally with GNU optimized and Clang ASan/UBSan. Both PASS: 24576 round trips (all coordinates along complementary X/Y diagonal across six IDs), 1152 single-bit record corruption cases, 250 invalid IDs with valid CRC, all 12 presets, golden vector independently checked with Python binascii.crc_hqx. Local tested blobs header d1b41d63b3a2b6872d81792f46d31347038dd3ce and test acf9665e44061d6b54fee08490891e2e386514a1. Commands:
```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/preset_test.cpp -o /tmp/preset_test
/tmp/preset_test
clang++ -std=c++17 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tests/preset_test.cpp -o /tmp/preset_san
/tmp/preset_san
```
Existing narrow workflow extended. Serial integration and exact-head target build still require CI. Local Git DNS and PlatformIO unavailable. No physical audition, storage backend, flash approval or measured on-device performance.

Original code. Re-read Diviner MIT LICENSE and hal_cyd.cpp at 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README at 495a98ca12bae467a507031f04931aa4254b1866; FM-1-MHAC README at 16c698d9e244559064a9a9a22f834d36912bf498. No upstream code copied or licensing changes. Named settings are creative musical configurations, not scientific results.
