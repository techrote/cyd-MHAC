# CYD-VISUAL-002 — Orbit, Loom, and watchdog recovery

Refs #3. Exact audio parent 1fec8bc503806513dbfb71103d6b1dda97a90e3f; previous visual parent 8636680cf70ddc85236ffe81f50fc81b3fcd0ab5. Audio changes are taken unchanged, not cherry-picked selectively. The previous integrated visual run 45730369618 and updated audio run 45735658376 both succeeded; neither qualifies this new combined target.

Two original integer-only modes: Orbit (curved interference shells follow the gesture) and Loom (warped interleaved threads). Cycle all five modes with top-left; no extra framebuffer or heap. Both respond to actual peak and sampled phase, XY and UI time; neither is a physical signal plot or semantic decoder. Existing three shaders unchanged. Orange top-right mute indicator means an audio-owned watchdog trip was observed. Release the current gesture, tap mute control to explicitly unmute, release, then play. A newly observed trip force-mutes even if the UI had resumed publishing. BOOT and DMA-error mute behavior remain. The v1 publish signature is unchanged. No new upstream code or licensing claims; original formulas and UI changes.

Native GNU C++ local optimized -Wall -Wextra -Werror and ASan/UBSan passes:
    mkdir -p preview
    g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/visual_002_test.cpp -o visual002
    ./visual002 preview
    g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tests/visual_002_test.cpp -o visual002san
    ./visual002san

Exact tested blobs: audio 9b17e33ad666d3a152ade642c4e363d3a51cd0ef; visual 74cf9e3cd17175a08cd65dde7ba01361b328dda3; test 731fea38a6f12d0ee5dddd28810ceeda8e4604fa. Native replay renews stationary control for 160 blocks, stalls for 120 blocks, observes one real audio-engine watchdog trip, verifies held-touch rejection, explicit release/unmute, sound recovery, same-trip no-op and counter rollover. Five distinct hashes 666ec7aedec17d03 / a3e1edb8569a9929 / 756db1d9afd54e9c / 10cb9377c077b0e6 / b9e0427b1254a401, identical optimized/sanitized. Guard words and orange indicator passed at all three tiers. Actual PPM previews generated. New modes 200 mixed frames: 1.193 / 0.333 / 0.121 ms/frame (steps 1/2/4, host only; excludes SPI/audio concurrency). Strip storage remains 5120 bytes. CI retains both original regression and new five-mode replay; target compile needed. No local PlatformIO/SDL2 or Git DNS, no device flash, no measured ESP32 performance or physical audition. Adapter is not locally compiled; owner gate remains HOLD. Next: interactive preview, touch calibration, on-device underrun/stall qualification, then persistent FX/gesture mappings.
