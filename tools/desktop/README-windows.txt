CYD-MHAC — Windows x64 interactive audition candidate
Extract all files, then double-click Open CYD-MHAC.cmd.
No CYD, DAW, MIDI, network or Python required at runtime.
Start muted: top-right unmute; release; left mouse drag to play.
Top-left visual; bottom-left sound; bottom-center-left factory bank (16 sounds; tap x88–159 of 320);
bottom-center-right hold factory audition; bottom-right pitch scale.
Bottom palette strip (x208–255 of 320): Original / Ember / Verdigris / Silver.
Tap to cycle; release before playing. Trace retains its feature colours.
Bottom mapping strip (x256–295 of 320): Direct, Swapped, Fine relative, Note lattice, Rhythm bands.
Fine relative starts at last sound coordinates; drag at half sensitivity, lift and re-anchor.
Mapping changes cancel quotation; the ring shows sound coordinates, not the finger in swapped/relative modes.
A new touch still interrupts gesture quotation as elsewhere on the surface.
Last four factory sounds: Stone Bell, Hollow Gong, Copper Rain, Glass Pins.
Select does not strike; hold audition to sound after explicit unmute.
Top-center gesture quotation; Esc mutes and cancels; Q quits.
Window focus loss/minimize also mutes. Start at low speaker volume.
This is the actual C++ audio engine and RGB565 renderer on a desktop,
not an ESP32 hardware simulation. Dummy-driver smoke is not listening.
See source-sha.txt, sdl-source-sha.txt and SHA256SUMS.txt.
SDL2 static dependency: release-2.32.10 (5d249570393f7a37e037abf22cd6012a4cc56a71).
SDL license and inherited Diviner notice retained. No repository-wide
license or hardware flash approval is implied.

Fourth XY mapping: Note lattice. Tap the mapping selector three times from Direct.
Each of three horizontal octave rows has 12 note cells (Free or Chromatic) or
5 (Minor pentatonic). Move horizontally to change notes, vertically within a
row to sweep timbre. Crossing an octave row changes octave and resets local
vertical position. The Free scale is chromatic in this mapping only. Other
mappings retain their normal scale behavior; factory audition bypasses mapping.

Fifth XY mapping: Rhythm bands. Tap the mapping selector four times from Direct.
Choose Erasure Pulse (Four Marks / Missing Mark) for the rhythmic interpretation.
X controls pitch with the existing scale; the four Y bands select stable timbre
levels 0, 1/3, 2/3, full. The second motif onset fades as you move upward.
Four tiny marks on the left of each band show nominal onset gains, not a live
beat meter. Other patches use the same bands as stepped timbre. The audio
motif clock runs independently. Factory audition still bypasses mapping.

Ninth visual engine: Caustic. From the initial Resonance visual, tap top-left
 eight times (release between taps). Intersecting curved light sheets and a
 gesture-centred ring respond to aggregate PCM peak, phase and timbre. This is
 decorative, not a measured acoustic field. The sound selection is unchanged.

Eighth synthesis engine: Quotation. From Phase Morph, tap bottom-left seven
 times, releasing between taps. The paired tenth visual, Palimpsest, shows
 stylized echoes of the current cursor; it is not DSP history telemetry.
 Direct mapping: X pitch, Y historical voice depth (up to 70%). Hold and
 move for more than 500 ms of active audio to hear recalled gestures.
 Mute and deselection invalidate DSP history. This is separate from the
 top-center UI gesture looper. Experimental session-only engine; factory
 bank and storage formats remain unchanged. Factory selection returns to
 a legacy patch.

Eleventh visual engine: Cellular. From Resonance, tap top-left ten times.
 A 40x30 excitable field propagates through a 16-state refractory cycle.
 Hold and drag to seed new fronts; measured aggregate PCM peak changes the
 excitation threshold. The visual clock is independent of the audio clock.
 Leaving and re-entering resets the field on the next observed frame.
 Decorative generative art, not a simulation of sound propagation.

Ninth session sound: Counterquotation. From Phase Morph tap the bottom-left
 selector eight times. Palimpsest adds crossing green contours to distinguish
 this variant; the contours are artistic, not a readout of stored notes.
 X remains pitch and Y historical blend. The recalled gesture alternates
 octave below and fifth above; wait at least 500 ms of active synthesis.
 The factory bank remains 16 entries; this variant is session-only.

Twelfth visual engine: Diffusion. From Resonance tap top-left eleven times.
Hold and drag to seed the reaction-diffusion field; aggregate PCM peak
modulates its evolution. This is generative art, not acoustic telemetry.

Six session scenes: tap the six green/grey bars at bottom x40–87 (release
between taps). One tap selects sound, visual, palette, mapping and scale,
without sounding or unmuting; next tap cycles. If you customize the scene
by changing one of those settings, the next scene tap starts at Lantern Tide.
Selection cancels the gesture loop and clears the factory cursor.
Factory audition remains a separate sound-only function.
1 Lantern Tide — Harmonic Lattice / Diffusion / pentatonic lattice.
2 Cinder Clock — Erasure Pulse / Cellular / rhythm bands.
3 Silver Bells — Modal Chime / Chime / pentatonic lattice.
4 Memory Crossing — Counterquotation / Palimpsest / chromatic XY.
5 Afterwater — Recursive Echo / Ink / fine relative XY.
6 Dust Prism — Ring Dialect / Caustic / swapped XY.
These are session recipes; nothing new is stored in flash.
