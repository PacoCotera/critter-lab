# Lab physical-design exploration

**Current direction:** the [owner physical-experience interview](../../specs/devices.md#owner-physical-experience-interview--current-direction) supersedes the printer-in-handheld case and fixed lower-row layout below. Develop a two-thumb home handheld, playable in its tidy shared caddy, with the printer in that home station. Field Lab is the original concept's name; Field Instrument and Orbital Lab remain inspiration. Retained boards below document earlier exploration, not the new configuration. Next visuals must show both handheld use and the supported living-display/play arrangement.

## Handheld and home station — architecture divergence

**Owner selection:** carry Contour forward, not its sculpted handles. Replace the narrow-waisted gamepad body with a substantial rugged Lab that belongs with the Probe and Companion and their protective bumpers. Small-batch 3D printing, straightforward owner assembly and obtainable controls govern convergence. Yoke and Keel remain exploration references and are not proceeding. No manufacturing feasibility is established by these renders. The next proof is a simple enclosure/component arrangement and assembly approach, followed by a model-based appearance study; preserve the dominant screen and shared printer station.

Construction hypothesis: broad softly rectangular body with usable edge thickness, separate front/rear shell parts and accessible standard fasteners; removable protective corner/side guards rather than integrated sculpted handles. Study control mounting, screen retention, print orientation, service access and supported-play clearance together. Printed flexible guards and sourced elastomer pieces are alternatives to investigate, not selected processes. Avoid introducing custom molded parts or a bespoke wheel mechanism merely to match a render. The compact station can retain the printer below the Lab and adjacent portable rests, provided the control and hand approaches remain clear. This brief is not CAD or fit evidence.

First-build priorities: accessible assembly using common tools, room for project-designed/sourced electronics, larger boards and wiring, and straightforward reopening. Do not optimize away connector access, cable routing or assembly clearance to preserve a thin silhouette. Enclosure dimensions follow the actual electronics layout. Later revisions may improve density. The next layout must show component envelopes, board mounting, wire paths, shell opening and tool access together; it must not claim manufacturing readiness from exterior art.

The first handheld board was rejected: its three proposals were too toy-like and too similar in physical form. The former common upper-shoulder Zoom placement is withdrawn; no top-mounted knob. Preserve two-thumb use and control functions while exploring placement. The [earlier board](handheld-home-directions-v2.png) remains a reference to rejected exploration, not a baseline.

Three new visual hypotheses compare the same journey: pick up the home Lab, explore with physical controls, return it to the shared printer station, continue playing while supported, then leave rotating collection/vivarium screens visible. These are concept renders, not dimensionally verified models. No new UI, creature, charging method or hardware is approved.

| Proposal | Handheld architecture | Station relationship | Main uncertainty to test after visual selection |
| --- | --- | --- | --- |
| Contour | Closed continuous body, narrowed waist and lower thumb shoulders | Compact drawer-console with open grip space and portable pockets | Lower control arc reach and palm support |
| Yoke | Screen between separate-looking integral side grips with visible air gaps | Freestanding support over a horizontal printer/portable tray | Total width, handle clearance and rigidity |
| Keel | Broad screen above one continuous full-width handbar | Low radial hub with printer and rear-side portable pockets | Central workspace-key reach and balanced handheld support |

Core functions remain directional navigation, Cancel-left/Confirm-right, four workspace shortcuts and inspection/zoom. Proposed placements are exploratory. Recessed side/end wheels replace a top knob. Screen artwork is a placeholder; these images do not change the approved Lab visual baseline. Printer belongs to the station, not the handheld. Printing while undocked is still open.

A physical mock-up must establish thumb reach, workspace-key identification, support stability and finger clearance while docked. Concept images cannot establish comfort, screen aperture, battery runtime, fit or material performance.

![A — Contour](architecture-a-contour.png)

![B — Yoke](architecture-b-yoke.png)

![C — Keel](architecture-c-keel.png)

The [generation and correction prompts](architecture-prompts.txt) retain provenance for these built-in image-generation studies. Keel's final study uses visible front-rail workspace keys instead of the initial rear-paddle hypothesis. Station underside clearance remains unresolved in Contour and Keel; Yoke makes open grip access more visible. The screen and receipt illustrations are placeholders, not firmware output or approved specimen art.

Experience Design inspected all three exact final exports for character/architecture review: rectangular portables, four workspace keys, Cancel left of Confirm, side/end Zoom and station-only printing are visible. This is not an ergonomic pass. Contour's central keys require hand relocation; Yoke's lower right hand approach may conflict with the portables; Keel's rim remains close to the grip underside and wheel. Support stability, physical reach and clearances need model/mock-up evidence. Owner selected Contour subject to the rugged construction correction above.

Joint physical/interaction exploration, 28 September 2026. No winner, dimensions, selected parts, implementation or purchases. Cream/charcoal/orange identity retained; workspace accent colors are proposals. Research and Library are owner-named destinations. Expedition, Inventory, Collection and Creation keys are tentative shortcuts to existing intended activities, not approved top-level information architecture.

## Same short journey, four different feels

| Moment | Chromatic Desk | Microscope Cradle | Experiment Station | Modular Wings |
| --- | --- | --- | --- | --- |
| Choose genome record | Research key; arrows select; Confirm opens | Research side key; list rocker; Confirm | Research keypad; directions; Confirm | Research pod key; directions; Confirm |
| Inspect feature / zoom | Arrows select, Inspect opens free detail, large Zoom wheel; arrows pan enlarged view | List rocker selects discrete feature targets and records; Inspect opens the selected feature; Pan ball pans only the opened viewport, never selects a study; Zoom changes view scale | Inspect, Zoom dial; Reference dial browses known authored contexts | Inspect and Zoom on movable navigation pod |
| Review / start research | Confirm opens terms; fresh Confirm on explicit Start commits | Same reviewed Confirm boundary | Confirm selects and reviews without spending; separate Run stroke commits reviewed operation only | Same reviewed Confirm boundary |
| Discovery | Saved finding replaces unknown only when accepted; free inspection remains available in every concept | Same | Same; Run inactive outside a supported review | Same |
| Library and return | Library key opens knowledge workspace; Research restores sample/topic/focus/zoom | Side keys preserve same context | Keypad destinations preserve same context | Workspace pod preserves same context when relocated |

Workspace switching is read-only navigation, never spending or cancellation. A committed operation can continue while browsing another workspace; show truthful global operation status, do not steal focus on completion. Returning to an uncommitted review restores a draft, revalidates the actual topic and stock, and shows a current explicit Start-ready state before a fresh deliberate action. A Run stroke or held input outside that state is discarded, never queued for later commitment. Workspace identity and selected content focus need distinct visible cues. The drawn keys use words and colors. Icons are intended but not yet drawn; combined word/icon/color legibility remains to be tested and the final palette is open. Unknown sample information is not revealed by zoom or a Reference dial.

## Physical character and questions

1. Chromatic Desk: a 2x2 destination keypad, four-way pad, large nonpush Zoom wheel and Inspect/Confirm/Back. Nine actuator bodies, eleven discrete press/direction functions and one rotary axis. Enjoyment hypothesis: destination muscle memory and a generous examination wheel. Test adjacent-key separation, mirror layouts, one-hand reach and rotary sweep.
2. Microscope Cradle: a side destination column, exposed Pan ball, two-way list rocker, large Zoom and Inspect/Confirm/Back. Ten bodies when the rocker is one body; nine press/direction functions, two pan axes and one rotary axis. The ball lets a specimen move beneath a reticle; this is an input/view proposal, not a new game rule. Test palm support, list versus spatial movement, side-key reach and ball feel; precision is unmeasured.
3. Experiment Station: a tentative six-key destination pad, four-way navigation, Zoom and known-Reference dials, Inspect/Confirm/Back and recessed Run lever. Thirteen bodies; fourteen discrete press/direction/stroke functions and two rotary axes. Pleasure hypothesis: arrange an experiment, compare, then deliberately run it. Run is inactive except on an explicit supported reviewed operation. Reference changes only known authored viewing context, never specimen state, genotype or unknown result. Test role clarity, positive lever stroke, destination reach and panel footprint.
4. Modular Wings: movable four-key workspace pod, navigation/Zoom pod and fixed Inspect/Confirm/Back action bank. Same nine control bodies/functions as Chromatic Desk. The first trial uses movable mock controls, not an assumed wireless system, battery or docking protocol. Enjoyment hypothesis: personalize posture/handedness. Test stability and whether positioning helps; mechanical/electrical connections remain open.

## Smallest comparison

Use identical full-size screen content and the same five-moment journey with paper/card mock controls on an adjustable deck. Include representative secured base mass, printer outlet and roll-access keepouts. Try left/right handed, one/two hands, workspace switches during a draft and pending operation. Observe what feels inviting, memorable or satisfying; also note overshoot, mistaken actions, context loss, reach, screen occlusion, base slip/rock and paper interference. Inert controls and authored response cards can compare interactions; only real controls and hardware can establish torque, motion, durability or power. No ergonomic outcome is measured yet.

## Focused interaction review

Experience Design reviewed the actual PNG and comparison text. Corrections clarify Cradle feature selection, Station draft revalidation and fresh Run strokes, and the absence of drawn icons. The static sheet is acceptable as physical exploration only. Active workspace versus content focus, global pending feedback, restored context and actual operation guards remain interaction-prototype requirements; this is not a usability or implementation pass.


## Product concept art

[Instrument pitch board](instrument-pitch-v1.png) visualizes the four possibilities with the selected Lab screen as a reference. Generated concept art, not a control-count or mechanical specification: illustrative extra side dials, symbols and printed material do not add approved functions. Use the layout sheet and mappings above for intended input roles. Owner favors layouts/compositions 1 (Chromatic Desk) and 3 (Experiment Station), but rejects their enclosure shapes. Continue enclosure divergence before combining preferred attributes; neither illustrated case is a baseline.



## Control family — owner direction

Retain colored workspace keys, directional cross, orange-accented rotary knob and round action buttons. Owner-approved action order: Cancel/return arrow on the left; Confirm/checkmark on the right. No dedicated Inspect in the current study; Confirm opens the selected detail. Size and group the cross and frequent actions with deliberate finger clearance. These are size/priority requirements, not measured dimensions. Case form remains open. [Six enclosure directions](enclosure-divergence-v1.png) explore form only; shown small controls are superseded by this direction. Confirm/Cancel naming and context behavior need alignment with existing Back semantics before implementation.


[Control-family reference](control-family-v2.png) retains the preferred key, cross, round-action and rotary styling; its standalone Inspect and wide spacing are superseded by the compact study. It isolates controls from the unresolved enclosure, not a standalone accessory proposal. Rendered proportions are not measured ergonomics; workspace symbols are placeholders.

Hardware review of enclosure board: useful distinct form attributes; folio closure and knob clearance unproven, all printer volumes unallocated, transparent internals illustrative, supports/rails not established as tilt locks or handles. Capture preferred form attributes before convergence.



## Current arrangement exploration

Owner direction: four colored workspace keys in one horizontal row below the screen; directional cross left, Cancel/Confirm together in the middle, orange-accented Zoom right. No Inspect. The [workspace-row comparison](workspace-row-v1.png) explores A evenly distributed row, B centered strip with slightly forward action pair, and C raised workspace shelf. These are arrangement/render proposals, not calibrated mechanical geometry or a selected enclosure. The symbols remain placeholders. Cross-to-edge and knob-to-Confirm clearance require physical layout verification. No option selected.

## Interaction-led sizing

The [dimensioned whole-face reference](ergonomics.md) predates the new horizontal workspace row; its grid arrangement is superseded and its geometry is not a fit claim for the renders. It shows the display and controls together at one millimetre scale: a proposed 215 × 230 mm developed face, not an assembled case footprint. It removes Inspect, groups navigation and actions, and places a compact workspace grid and orange-accented Zoom nearby. Earlier 280/320 mm layouts were rejected and are not active sizing recommendations. Longer examination welcomes two hands; every sequence must also work with either hand alone. Physical comfort and internal packaging remain untested.


UX reviewed the actual workspace-row board: roles/order retained, no blocking composition defect for comparing concepts. A has clearest row alignment; B needs left-edge clearance checking; C separates roles but adds a reach-over shelf. Destination symbols need labels or a learned on-screen cue; the concept icons do not establish those meanings. Grip and comfort remain unmeasured.


## Current casing exploration

The Lab is one integrated assembly. Owner permits an asymmetric body with the screen/control group offset beside an internal printer and tap-electronics zone, or printing below the controls. The [single-body allocation study](single-body-allocation-v1.png) compares right service bay, lower printer bay and left service bay. Owner selected C (left printer bay) for refinement. The [README ecosystem reference](../references/ecosystem.png) anchors textured warm cream, charcoal screen surround, visible service fasteners and restrained orange accents; its old controls and wedge shape are not revived.

Reserve space for the roll, feed mechanism, cutter, output path and accessible roll-change door. Provide an approachable side tap surface for Probe and Companion with its own electronics allowance. Insets are unscaled conceptual reservations, not mechanical sections: module size, roll orientation/feed routing, cutter design, control back clearance, compute, cables, antenna/reader separation and service clearances remain unverified. Tap does not imply charging, authorization or automatic successful transfer. Cutter inclusion is a packaging exploration, not a selected automatic-cut mechanism.

The colored workspace row, cross, Cancel-left/Confirm-right and orange-accented Zoom remain the control family, with no Inspect. Screen and printed art in these generated housing concepts are illustrative, not revisions to approved UI or game semantics. Prior separate-volume/bridge arrangements and wedge renders do not satisfy the current single-body direction.

## Selected C refinement

Owner chose C with integrated left printer bay and requested a vertical workspace-key column at the far-right front edge, counterbalancing the printer and freeing room to raise the main controls. [C with vertical workspace keys](c-vertical-keys-v1.png) is the current visual review target. No horizontal workspace row remains in this revision. Cross, Cancel-left/Confirm-right and orange Zoom remain beneath the screen with more base clearance; no Inspect. The left-side tap surface is separate from front keys. This supersedes horizontal-row placement for the selected case, without changing the control roles. Rendered screen/paper art remains illustrative; case dimensions, grip clearance, printer and tap implementation are unverified.

## Screen prominence refinement

Owner approved A in the [screen-first comparison](screen-first-v2.png): narrow vertical workspace keys at the right front, prominent 7-inch screen, integrated printer bay on the left, Cancel-left/Confirm-right and no Inspect. This supersedes the earlier horizontal-row and smaller-screen review targets. Renderer proportions are not calibrated dimensions; generated UI text is illustrative.

The next outcome is printer and tap packaging within A. Owner favors the front-left panel above the paper outlet as a contrasting, clearly labeled TAP surface and permits rear roll access. Top or right-side tapping remain alternatives if this arrangement encounters a concrete packaging problem. Keep the tap panel fixed and distinct from the service door. Preserve screen prominence while resolving roll orientation, feed path, output/cutter allowance and service access; actual components and mechanical fit remain open. See [packaging study](ergonomics.md#selected-a--printer-and-tap-packaging-discovery) for sourced constraints and proposals.

Owner rejected the cross-section in [the retained packaging attempt](printer-tap-packaging.png); it is not packaging evidence or a valid mechanical arrangement. Establish an actual dimensioned layout from the printer's mechanical documentation before illustrating internals again. The front pad treatment is also rejected as too bold and generic: use a muted surface with recognizable Probe and Companion identity, rather than a large TAP label and payment-like radio symbol. Front-above-output placement and rear roll access remain the working direction. Rendered screen and paper graphics are incidental, not approved UI or specimen art.

Exterior reference: [A with muted Probe/Companion pad](prototype-a-muted-pad.png). Preserves the screen-first case and control order, removes the pad latch, and uses a quiet sage-gray inset with paired device icons and names. Rear loading remains intended. Screen and printed specimen are illustrative.

Owner approved exploring a slight backward recline of the screen and control face, provisionally about 15 degrees from vertical, to face upward on a tabletop. Keep the base flat, printer outlet more upright and the compact rounded single assembly; do not revive the rejected large wedge. Add two shallow delimiting grooves: vertically between printer/pad bay and main panel, horizontally between screen and lower controls. The angle is a prototype hypothesis, not a measured ergonomic optimum or a calibrated render dimension.

Rejected geometry reference: [reclined A with section grooves](prototype-a-reclined-grooves.png). Owner found the resulting form inconsistent. Retain the requested recline and delimiting grooves as design intent, not this generated enclosure geometry.

## Dimensioned model before enclosure renders

**Current exploration exception:** owner explicitly requested **visuals first** for five divergent enclosure concepts, drawing from different design traditions and varying shape, proportions, color and materials. These are loose visual proposals, not dimensional proofs. Preserve the agreed functional layout and screen prominence. After selecting attributes, return to the model-based workflow below for convergence and mechanical claims.

## Five enclosure directions — current visual comparison

![Five visual enclosure directions](five-enclosure-directions.png)

| Direction | Form and material proposal | Experience intent |
| --- | --- | --- |
| A — Field Instrument | Faceted graphite protection, mineral-gray face and exposed fastener language | A compact scientific tool with tactile protective edges |
| B — Radical Lab | Stepped architectural massing, vermilion and ultramarine body, yellow control ledge | An expressive, playful object inspired by Italian radical design |
| C — Precision Desk | Thin metal face, fine perimeter frame, cool gray and black | A restrained instrument whose controls supply the color |
| D — Bio Workshop | Rounded ash-like wooden cheeks, forest-green face and warm metal details | A welcoming craft object for biological discovery |
| E — Orbital Lab | Continuous capsule shell, silver, midnight blue and lime accents | A friendly space-age tabletop instrument |

The fixed layout survives across all five: dominant landscape screen; left printer output with portable-presentation area above; four right workspace keys; cross, Cancel-left/Confirm-right and Zoom below. Rear loading remains a requirement but is not shown. The board compares visual character, not exact scale or component fit. Depicted metals/wood need material, reader and manufacturing evaluation before selection; screen and printed art are illustrative. No winner selected. Capture preferred attributes before making a dimensioned model; do not infer a final enclosure from this image.

Brief references include [HIOKI's compact field instrument](https://www.hioki.com/in-en/products/testers/compact/id_5844), [Triennale Milano on Memphis](https://triennale.org/en/magazine/a-graphic-story-about-the-italian-design-collective), and [HfG-Archiv Ulm](https://www.hfg-archiv.ulm.de/). These inform design principles rather than authorizing copies or defining a national style.

## Model-based convergence after selection

Owner requires subsequent enclosure renders to originate from a dimensionally and materially coherent model. Use a single editable model in millimetres for every exterior, section and service view. Generated images may inform style, but must not establish or modify mechanical geometry.

Model the display module, glass and visible aperture as distinct boundaries; use verified manufacturer dimensions where available. Mark proposed housing dimensions, control caps/back clearance, printer mounting/service volume and reader allowance as proposals. Keep one coordinate system and explicit face angle. Resolve transitions between the reclined main face and printer bay, wall thickness, corner radii, groove width/depth, rear access and base contact in geometry. Material assignments belong to model surfaces; shading must not invent recesses or seams.

Before a beauty render, inspect front, side, top and section views from that same model, check component intersections and opening/service paths, then obtain focused independent hardware review. Publish the model, parameter/source table and matching exports together in Git. A dimensional prototype is not manufacturing-ready CAD or proof of ergonomic, thermal, RF or printer performance. No further generated-image approximation of the case is a substitute for this step.
