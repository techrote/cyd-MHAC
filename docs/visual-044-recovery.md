# VISUAL-044R — mode-cycle regression recovery (refs #3 #4)

Parent 09724188d250766a0a95d05238e0b6d3b32b75ca. Run 46089015100: Windows PASS; Linux native replay FAIL at tests/caustic_study.cpp:50 (stale assertion that Diffusion is followed by mode zero). Desktop smoke and target compile skipped. Preserve failed run and artifact 15239948271; do not treat it as a firmware package. Last same-source qualified pair remains VISUAL-043, run 46083027839.

Biolume was appended after Diffusion, ID 12. Update Caustic, Cellular and Diffusion study tails to assert the additional step explicitly before wrap to zero. Retain all prior checks; no runtime, DSP, UI, hardware, or CI configuration changes. The later CI tests were inspected for stale mode-wrap assumptions; the full thirteen-mode replay already checks the count and all modes.

Extend the existing Diffusion study's host render timing to compare Diffusion and Biolume at all three quality tiers, using the same frozen pigment, audio snapshot, controls and tick sequence. Check strip sentinels and all 768 cells remain unchanged after each batch. 180 frames per mode/tier; no absolute timing assertion. These are host renderer timings only, not physical ESP32 or concurrent audio evidence.

Local clone unavailable (GitHub DNS resolution fails); source review and exact-tree verification only before publishing. Existing integrated CI must execute the full suite and compile the target. No new jobs. Draft/HOLD, no main merge or flash authorization. Next audio lane remains offline prepdist interval review.
