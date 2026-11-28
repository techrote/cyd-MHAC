# VISUAL-044 — Biolume (refs #3 #4)
 
Parent ea4de7027486ec016cc46141d0ec9e7ada78ab03, VISUAL-043 both jobs SUCCESS run 46083027839. Windows 15238340331 and target 15237801698 are nonexpired exact-source metadata, not downloaded this run. Audio #2 unchanged at 6db2b302034d4253d7f1f115de7e41b923560449. 
 
Append visual ID 12, Biolume. The existing Diffusion pigment gates narrow animated light sheets, with pigment-dependent refraction, intersection flares and aggregate PCM peak modulating illumination. This is generative art, not spectral analysis or direct research implementation. Both modes share the existing canvas, with no second simulation or framebuffer. Existing mode IDs, scenes, audio, pins and quality tiers are unchanged. Touch selection uses existing top-left cycle. Palette, mapping, cursor and safety overlays remain in their common pipeline. The shared field evolves while either mode is selected and pauses otherwise; mode selection closes gate as before. No new persistent allocation. Additional per-pixel integer work; physical timing unmeasured.
 
Executed isolated shader tests with synthetic pigment (not live diffusion or full UI):
 
    g++ -std=c++17 -O2 -Wall -Wextra -Werror -Iinclude tests/biolume_test.cpp -o /tmp/biolume-gcc && /tmp/biolume-gcc gcc-
    clang++ -std=c++17 -O1 -g -fsanitize=address,undefined -Wall -Wextra -Werror -Iinclude tests/biolume_test.cpp -o /tmp/biolume-san && /tmp/biolume-san san-
 
Both PASS 356352 bounded/deterministic shade checks each; four 320x240 RGB565 quantized PPMs byte-identical across compilers. FNV hashes eb3cd21fe15444b0, 0ca97a33a1d635ab, bbe106ba50af007c, ee50b798f3331cc2. Tested Git blobs shader 6ac6ec71f2f07781fb62975f364f158174b4fec4, test 7c2455df06a48980a6d45601ba3abce98dfcdf2f. Integrated existing replay extended to 13 modes with actual shared diffusion pointer and safety/strip checks; CI pending (local GitHub DNS unavailable). No extra CI jobs or dependencies.
 
Read-only reference review Diviner MIT license and README 82e06e11e5947e41e888716e7a68e99187410f2e; MHAC README 495a98ca12bae467a507031f04931aa4254b1866; FM-1-MHAC README 16c698d9e244559064a9a9a22f834d36912bf498. Original shader, no upstream code copied. Draft/HOLD, no merge or flash approval. Next: integrated qualification and real field captures; audio next offline prepdist analysis then creative synthesis.
