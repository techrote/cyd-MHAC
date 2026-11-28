# VISUAL-034 — pitch-only retrograde (Refs #3)

Draft/HOLD. Visual parent 146dddaa88d73ec7aebf90892e864c1ecf891717; AUDIO-034 7e514c1410d359bfe6fd9f43b7739ef50a381bc1 run 45981558589 success. Imported its three changed blobs unchanged. Previous VISUAL-033 run 45975326693 success, Windows artifact 15186308618, ESP32 artifact 15186816988, nonexpired source-matched metadata (not independently downloaded this run).

Original reversible pitch-only transformation: x272–295 y0–23, two opposing arrows. One tap reverses the recorded pitch values at their existing timestamps; second tap restores them exactly unless intervening pitch edits. Not a full time reversal or undo history. Keeps timbre, gate/rest, playback phase, scale-allowed pitch set and transport; disarms overdubs. Uses existing buffer, at most 125 swaps per fresh tap, no added state or audio-path work. Stale captured contact mutes and stops. No new dependency or audio wire change.

Executed core against blob-matched parent d78f75499fa7f709ea3aa19070bdf8976182a6f5:
```sh
g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/retrograde_test.cpp -o test && ./test
clang++ -std=c++17 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude tests/retrograde_test.cpp -o test-san && ./test-san
```
Both PASS: 72 profiles, 370368 exact comparisons, 152444 changed pitch observations, loop 1516 B. Six transport styles, 1/2/3/20/250/251 marks, two clock origins including rollover; independently constructed reversed reference and double reversal. Tested candidate header bb6822cdcee9ffc72772f2cec80573f470a8a91b and test 80f1b0cb3935137464ae0a66907531c451b86fd5. Integrated UI test and RGB565 marker await existing CI; no new jobs, target measurements, physical audition or flash approval.

Read-only Diviner 82e06e11e5947e41e888716e7a68e99187410f2e theme, hal_cyd, hal_native, MIT; MHAC 495a98ca12bae467a507031f04931aa4254b1866 README; FM-1 16c698d9e244559064a9a9a22f834d36912bf498 README; Trace Bell 4f528f7496ab58f44ba7ffc5bd6b570ecbcda112 VST3 docs. Original creative exaptation, no upstream source copied, no perception or formal research claim. Next: exact integrated build and owner audition.
