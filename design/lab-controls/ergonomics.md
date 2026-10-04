# Dimensioned whole-face reference

The workspace-grid arrangement below is superseded by the [horizontal-row exploration](concepts.md#current-arrangement-exploration). Its dimensions remain reference hypotheses; they do not dimension the new renders.

28 September 2026. Current proposal for review; no dedicated Inspect. Nominal **215 × 230 mm flat whole-face template**; screen and controls use the same millimetre scale. This is a developed front/control-face study, not an assembled enclosure, projected tabletop footprint, volume or proof of internal fit. Case shape is open.

[View the layout](compact-whole-face.png) · [Full-scale editable template](compact-whole-face.svg)

## Geometry

Origin: upper-left of the whole flat face. The selected H module maximum envelope is 164.90 × 124.27 mm at (25.05,10). The manufacturer front-glass rectangle is 164.90 × 106.96 mm; its placement in the module outline is illustrative rather than a mechanical mounting definition. No active pixel aperture is inferred or filled with game pixels. Source: [manufacturer H Rev4.1 dimensional drawing](https://www.waveshare.com/img/devkit/LCD/7HP/Exterior-Size.jpg), inspected in the preceding pass; rear photograph explicitly identifies H. Board, front glass and active pixels are different boundaries.

| Control | Centre / top-left, mm | Mock target |
| --- | --- | --- |
| Workspace grid | key top-lefts (12,158), (38,158), (12,184), (38,184) | 20 mm square caps; 26 mm pitch, 6 mm edge gap |
| Direction cross | centre (90,185) | 40 mm overall span |
| Cancel LEFT | centre (128,207) | 24 mm diameter |
| Confirm RIGHT | centre (158,207) | 24 mm diameter; 30 mm pitch, 6 mm edge gap |
| Orange-accented Zoom | centre (187,174) | 36 mm diameter |

The workspace grid has a consistent rhythm. The cross sits beside the primary actions; the dial is separately recognizable but near Confirm. Removing Inspect eliminates the duplicate action and frees a position rather than enlarging all remaining controls to fill space. Workspace grid to cross has 12 mm nominal horizontal separation; cross to Cancel cap has 6 mm horizontal separation. Primary cap front margin is 11 mm, not a claimed palm rest. The nominal Zoom cap has 10 mm to the right face edge and about 13.9 mm cap-edge separation from Confirm. None of these distances proves finger or whole-hand grip clearance.

Calculated centres: cross→Confirm 71.5 mm, cross→Zoom 97.6 mm, Confirm→Zoom 43.9 mm. These are diagram distances, not comfortable reach, timings or a planted-thumb envelope. Both single hands must operate serially by repositioning; two hands may divide navigation from dial/actions. No chords or simultaneous controls are required. The 215 mm width is a spatial proposal closer to the approximately 165 mm module, replacing the unsupported starting assumption that a much wider control deck was needed.

## Same play sequence, fewer unnecessary steps

Research key → cross selects record → Confirm opens → cross selects known feature → Confirm opens free detail → optional Zoom/cross pan → Cancel restores feature → cross selects a supported study target → Confirm opens its study review → fresh Confirm on explicit Start commits → saved discovery → Library key → Research restores context. Inspection and zoom remain optional, not mandatory preparation; discovery requires no gratuitous acknowledgement. Cancel is return/leave-review before commitment, not implicit cancellation of a submitted operation. Workspace keys preserve context and never spend or cancel. E*/I* are tentative destinations, not approved main-screen names. No physical Inspect shortcut remains.

## Review boundary

PNG is a legible preview. SVG is editable and dimensioned in mm; print at 100%, no fit-to-page, and check its 100 mm line before using as a full-scale paper mock. The diagram shows the module envelope and controls together at one scale, not measured manufacturing or ergonomics. Screen tilt, control-face slope, housing height/depth, cap forces/travel, knob grip, printer mechanism/roll/feed/tear edge, scanner, computer, connector exits and service space are unallocated. No printer slot or internal fit is invented to make the geometry appear solved.

Hardware inspected the actual PNG for scale, spacing, symbols and captions. Experience Design reviewed the actual final PNG and accepted the whole-face composition study: regular grid, coherent cross/action cluster, correct action order, separate nearby Zoom, no Inspect and adequate caption clearance. Its play-sequence correction is incorporated: select a supported study target before opening paid review, rather than changing the meaning of the same known feature. No redraw requested. This is not an ergonomics or internal-fit approval; dial grip, 13.9 mm cap gap and 11 mm front margin need physical testing. No final size selection.




## Selected A — printer and tap packaging discovery

28 September 2026. Design review accepted **A, slim right-front workspace keys**, in [screen-first-v2](screen-first-v2.png). Preserve the single integrated body, left printer bay, prominent selected 7-inch H display, cross, Cancel-left/Confirm-right and orange Zoom; no Inspect. The earlier whole-face/grid geometry above is historical reference, not dimensions for this case. This section is one packaging proposal for a cutaway, not a part selection, final fit, antenna design or purchase.

### One sourced mechanism example

Use **Seiko Instruments CAPD245** as a dimensional example, not the selected printer. Its manufacturer [product specification](https://www.sii.co.jp/sps/eg/product/lowvoltage/capd245.html) and [manufacturer datasheet](https://seiko-instruments.de/wp-content/uploads/2023/04/capd245-345_screen.pdf) state a built-in slide cutter, curved paper path and **83.1 × 35.4 × 26.9 mm (W × D × H), excluding the mounting part**. Paper width is **58 mm**, printable width **48 mm**. Those widths are not interchangeable and do not include a roll or driver board.

The manufacturer's [recommended-paper table](https://www.sii.co.jp/sps/eg/product/paper1.html) lists **TP-322L** for CAPD245: **58 mm paper width, 30 mm external roll diameter, 9 mm internal diameter**, with no separate core indicated. Thus the small roll is nominally a 58 mm long cylinder with 30 mm diameter; the 9 mm hole is not a required spindle specification. This is one recommended small-roll example, not the Lab's selected capacity or consumable.

### Left-bay allocation and path

Start the allocation study with a **100 mm wide × 95 mm deep × 90 mm high internal mechanical reserve** for the roll, mechanism/cutter, brackets and an accessible paper route. This deliberately rounded reservation is our hypothesis, not a manufacturer envelope or established fit. The example mechanism's mounting parts, latch movement, fasteners, guide geometry and service access may require it to grow. Reserve printer driver/power electronics and connectors separately; they are not magically included in the mechanism dimensions or this mechanical fit claim. No total case size is set.

Keep the roll axis **left-to-right**, parallel to paper width and to the front output slot. Place the roll above and toward the rear of the left bay, with the printing mechanism/cutter nearer the front/lower area. Show the strip leaving the roll tangentially, entering the manufacturer's curved print path, passing head/platen then the integrated cutter, and exiting the **left-front face**. Orient the mechanism so its supported discharge faces that outlet; the cited product summary does not establish mounting orientation, guide radii or the exact installed path. Do not add a tight turn or rotate the roll axis to pretend a narrow bay can fit. A cutaway path is a routing proposal; confirm it against the detailed mechanical reference before CAD or integration.

Use a **rear roll hatch** for loading, independent of the fixed front TAP pad and display/control assembly. Show the actual threading view and access to the platen release; a roll-only opening does not establish jam access. If the front mechanism cannot be serviced through the rear, reserve a separately labeled lower-left side service opening for platen/jam access, not a latch on the TAP pad. Its geometry and blade isolation remain unresolved. Intended reload: clear hanging output, open hatch, lift/drop the roll and lead paper through the actual opened path, close/latch, then perform a supported feed/status check. Brief two-hand servicing is acceptable; ordinary play remains single-hand serial. Show rear hatch swing and hand access clear of cables, wall/desk objects and internal components; test whether it can be used in place or needs deliberate safe repositioning. The fixed front pad does not move for loading. Remove decorative front-hatch/latch cues from that pad. Jam access must not require reaching through an exposed cutter; blade guarding, isolated service and actual latch/interlock behavior remain unresolved. Paper output stays beside rather than through the playing-hand area. A full/partial cut feature in the example is not approval of a Lab cut policy or safe implemented cutter.

Width reality: this candidate's **83.1 mm mechanism** is already wider than its **58 mm paper**. Adjacent to the H module's **164.9 mm maximum width**, those two hardware widths alone sum to **248.0 mm**, before mounting, walls, separation and right-front keys. This is a conditional dimensional sum for this arrangement, not a minimum for every possible printer or a final overall width. Preserve display scale; use actual bay reserve rather than shrinking printer hardware in art.

### Front-left TAP and rear loading — latest design direction

The preferred placement is now a **fixed front-left pad above the paper outlet**, using a muted surface and modest recognizable Probe and Companion silhouettes, optionally with small device names. This supersedes top-right-first exploration. The pad is not the roll hatch, a print button, latch, charger or proof of transfer. Do not draw a decorative orange latch/service seam on it. The rear hatch is the preferred roll access. Top/right alternatives are held only for an evidence-backed packaging blocker, not competing default designs.

Show a distinct **unquantified tap-reader/electronics reservation behind the front pad**. It must not occupy the moving paper path, roll-change corridor or cutter/platen access. The nearby head, motor, cutter metal and roll holder are real design constraints for a possible antenna/reader. Technology remains open, so there is no defensible RF spacing, wall thickness, reader-board envelope, antenna field or detection range yet. Do not hide a fabricated module between the roll and housing or claim that this pad works through metal. Reader/antenna selection and representative detection/interference tests must resolve that boundary before fit or reliability is asserted. Printer power/driver electronics need their own space and wiring route, separate from this pad reservation.

Hardware and UX agree on the presentation sequence: approach the fixed pad gently with either Probe or Companion, keep hanging paper and the outlet clear, see recognized device/player and the supported next step on the screen, then withdraw the portable before operating controls where the eventual transport permits. No pressure, resting dock, hold-through-Confirm or charging is required by this proposal. Recognition does not confer ownership or prove an accepted receipt. Supported intent/manifest review and truthful pending/result remain on the Lab; a tap alone never produces Probe Empty/Ready or cloud acceptance.

Both portable-plus-hand envelopes must clear the front paper, printer mouth and right-side control group while leaving acknowledgment visible. The pad's depicted rectangle is not yet a usable footprint. Its accessibility and technology cannot be proved by a case render.

For reload, finish/remove output, open the **rear hatch**, load the roll and lead paper through the visible actual path, close/latch, return to the screen and run a supported feed/status check. Brief two-handed service is acceptable; ordinary play remains serial with either hand. Rear approach space and cable management must be explicit. If the rear opening does not expose the platen or a jam, show the separate service-access reservation rather than pretending roll access solves every printer fault. Guarding and isolated cutter service remain unengineered.

### Rejected cutaway and required mechanical proof

Design review rejected [the generated packaging image](printer-tap-packaging.png). Hardware and UX acceptance is withdrawn. The bold TAP/payment-style treatment is also rejected. Front presentation above the outlet and rear roll access remain working directions; the illustrated internals are not a valid design basis.

The drawing did not derive its mechanism orientation, inlet, head/platen/cutter geometry or service release from a manufacturer mechanical reference. Its views were not constructed from one dimensioned model. A continuous illustrated strip and corrected arrows do not establish a usable loading path. Labeling these omissions schematic did not make the packaging coherent.

Available evidence establishes the example mechanism envelope and a recommended roll size only. The next proof is a dimensioned side and rear layout using the manufacturer's mounting, paper-entry/exit and platen-release documentation, with a shared coordinate system. It must show the roll axis and tangency, supported mechanism orientation, mounting and electronics allowances, rear loading reach and an actual jam-access route. Keep unknown dimensions visibly unresolved. If that documentation is unavailable, retain an envelope study and explicitly stop short of a mechanical section.

Do not generate further internal concept art before this geometry is established. A separate exterior refinement can use muted paired device silhouettes from the original Probe/Companion references. No final printer, reader, cutter policy, case dimensions or purchase is approved.
