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
