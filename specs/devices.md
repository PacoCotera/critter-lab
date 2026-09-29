# Devices and physical interfaces

Status: intended functions and engineering constraints. No selected production parts, board layout, enclosure dimensions, thermal margin or battery runtime is established.

| Device | Required role | Open physical/firmware choices |
| --- | --- | --- |
| Lab / Console | Home handheld and supported tabletop research workbench; Wi-Fi, local configuration, cached archive, color pixel display, wireless print requests and QR reading; play without a Probe | Selected development display: Waveshare 7inch HDMI LCD (H), 1024×600 landscape color; Linux host, drivers, two-thumb controls, battery and scan remain open |
| Companion / combined portable | Primary everyday interaction device; Probe, Cargo and Companions modes combine expeditions/sensing/capture with carried findings and critter training/interaction | Combined display, compute, sensors, battery, controls, carried-critter and containment capacity require reevaluation; no separate Probe hardware in the current concept |
| Caddy / home habitat | Charges Lab and Companion, prints records and presents habitats, residents, eggs/embryos/incubations and environmental/population state | Habitat display, charging, printer packaging, local cache and synchronization remain open |

The [physical-experience principle](experience.md#physical-experience-is-the-product) defines why these are distinct objects: simple e-ink collection, responsive companionship, an extensible Lab workbench and a physical charging home. It guides engineering exploration without selecting production modules.

Warm beige shells, charcoal structure and restrained orange accents inform the product family. Concept proportions and earlier renders do not freeze controls, ports, materials or manufacture. No physical slots are implied by virtual Lab chips.

## Development targets

### Current consolidated product architecture

Owner approved a three-object system: **one everyday Companion, one home handheld Lab, and one caddy/printer/habitat station**. This supersedes separate Probe and Companion hardware. Existing Probe gameplay and sensor work become Companion functions; retain their software/reference evidence rather than discard it. Legacy board/display targets below describe existing development work, not a frozen selection for the combined portable.

The Companion is the most frequently used device and gets priority in interaction, portability, expressive display, control comfort and power budgeting. Its switchable modes are **Probe** (field observations, expeditions, gathering and wild encounters), **Cargo** (carried resources, samples, items and separately identified temporary wild captures), and **Companions** (established travelling critters, interaction, training and development). Switching views must not implicitly stop gathering, spend resources or discard an encounter. Field investigation does not replace Lab genome decoding.

The Lab handles research and entry to the vivarium/habitat. The caddy is the tangible home or "memory" of the world: residents and environments such as tanks, islands or glaciers, with environmental parameters and populations. Durable shared state is cloud-authoritative. A synchronized caddy cache/projection is proposed; exact offline behavior and local authority are not decided. Lab, Companion and later a mobile app access the same world. The caddy does not become a second independent care simulation or an assumed mandatory network gateway.

The accepted wild-capture direction and its open mechanics are recorded in [gameplay](gameplay.md#combined-portable-and-wild-capture). Keep temporary captures distinct from bonded travelling companions and from decoded genomes in all device views.

#### Coordinated docked defaults

Family branding: every device reads **Critter Lab**. The caddy front uses the compact **Critter Lab by DPP** mark; Dirty Pawz Press remains its underlying brand, with full identity on service labeling/printed material rather than a crowded front wordmark. Companion retains its secondary device designation. Final typography/emblem artwork remains a visual proposal.

Owner-authorized caddy control concept: Previous/OK/Next beside the summary display; arrows browse/move focus, OK opens or confirms, and detail views provide a visible Back choice. Print beside the paper slot opens a preview with Print/Cancel choices confirmed by OK; subordinate recessed Feed sits directly below Print. Remove arbitrary standalone caddy LEDs. Charging and separate network/cloud statuses belong on the summary display; add a labeled bay light only if physical testing identifies a need for immediate feedback. Exact confirmation flow, feedback latency and electrical support are not implemented or validated; consequential habitat operations remain on the Lab.

Owner direction: the three docked displays form one integrated experience with distinct responsibilities. **Lab** is the most visual: living environments, creatures, incubations and ongoing research. **Caddy/habitat** provides the persistent summary: stored residents, habitats, bulk inventory, device charging, network and cloud connection status. E-ink is a candidate for this summary display; exact technology and color capability remain open. **Companion** defaults to an always-on Probe role, scanning and gathering even while docked. This supersedes the earlier caddy living-scene/matrix proposal; the Lab owns that visual presentation.

These are default views, not locked modes. Charging must not silently stop gathering. Exact stationary gathering rates, capacity behavior, sensor validity during charging and offline operation remain design/engineering work; no automatic wild capture is implied. Status must distinguish fresh, stale and unavailable information rather than display an assumed successful charge or cloud connection. The three views refer to the same world and must not duplicate independent inventories or incubation state.

Current concept exploration may revise the combined portable's enclosure/display/controls; existing simulator inputs and firmware targets are unchanged until an approved implementation slice. No purchase or new PCB is authorized by these proposals.

### Owner physical-experience interview — current direction

The Lab invites pickup and exploration at home. It is a two-handed handheld with a Switch-like grip and thumb access to core controls; brief one-hand support while the other turns the inspection/zoom knob is acceptable. A larger body is acceptable to preserve screen and control comfort. The former fixed lower control row and printer-in-handheld enclosure are superseded constraints, not approved ergonomic geometry.

Move the printer into the caddy/home station. Keep all three devices together in one tidy, clean place, preserving the original **Field Lab** concept's character. Whether the home station is one shell or coordinated adjoining sections remains open. The Lab must remain comfortably playable while supported there, and be charged and ready when picked up. Typical handheld sessions are about 30 minutes; this is a use case, not a measured battery-runtime promise. Charging technology and power budget are unselected.

Probe and Companion interact by tapping the Lab; inter-device communication is wireless. Owner does not want custom data connectors. Do not infer a charging connector, radio protocol, tap-reader technology or completed transfer from that preference. Wi-Fi is required for the home Lab. Whether wireless printing requires the Lab to be seated in the station remains undecided.

At rest in the home station, the Lab is a living display: collection and vivarium occupy rotating full screens. Ambient light governs automatic dimming. Rotation timing, brightness curve and behavior on interaction remain to be designed; no new screen UI or sensor part is selected by this interview.

The next visual divergence follows this handheld-plus-home-station experience. **Field Lab** names the warm original concept; Field Instrument and Orbital Lab remain selected sources of inspiration. Visual-first exploration is authorized, followed by model-based geometric development after attribute selection. The first handheld comparison was rejected as toy-like and insufficiently divergent. A top-mounted knob is excluded. Two-thumb use does not prescribe a Switch-shaped enclosure: alternatives must differ in physical architecture, grip, control placement and station relationship, not merely color. Retain control functions while exploring their placement.

Owner selected **Contour** as the promising architecture for convergence, with a substantial correction: remove its controller-like handles/narrow waist and recover the rugged, bulky family character of the Probe and Companion, including protective bumpers. Small-batch 3D printing and owner assembly are primary constraints. Yoke and Keel are not proceeding because their construction/parts burden and family mismatch do not suit this project. The selection does not approve the rendered shell as manufacturable. Establish simple shell separation, fastener access, display/control retention and feasible bumper construction before refining appearance; bumper material and fabrication method remain open.

First-build packaging must accommodate electronics sourced and designed by the project, including larger PCBs, connectors and discrete wiring where needed. Dense integration is not a first-iteration goal. Assembly, reopening and service must be straightforward with common tools, with room to route and disconnect wiring and reach fasteners. Size the enclosure from actual component and wiring envelopes rather than forcing electronics into render proportions. Space optimization belongs to later iterations; additional volume is acceptable, subject to the handheld and supported-play requirements. Preserve the settled screen-dominant front proportions: roomier electronics do not authorize widening the face around the display. Explore depth and component arrangement first; any unavoidable width tradeoff must be made explicit.

Owner supports the rugged treatment but requests distinguishable device roles: branded home Lab with a potentially different case color; Probe and Companion retain portable character. Workspace keys need recognizable purposes beyond color. Explore a V1 face without the exposed knob; omitting it is a proposal until its interaction consequences are resolved, not an instruction to remove the existing simulator input. Companion must read as a substantial distinct device, and station spacing must permit pickup rather than crowding the devices. Reduce perceived caddy bulk while retaining honest printer/service reservations. Branding and status lights are candidates; the proposed small display has the habitat purpose below, with no hardware or implementation selected.

Owner subsequently gave the caddy display a product purpose: the **home habitat**, showing eggs, critters and their living environments. Explore a small persistent living-scene display in the station; Lab remains the research/management device. This is an experience direction, not selection of an LED matrix/panel, local storage authority, incubation rules, care mechanic or firmware architecture. The relationship to Lab's collection/vivarium views, off-dock behavior and synchronized state must be resolved before implementation.

Owner direction: a more capable Linux-class Lab, an nRF52840 Probe and an ESP32-S3 Companion. The Lab display and Companion board/display below are selected for preliminary development; the Probe panel remains a candidate. Sensor sets, battery configurations and production parts remain open. Compare Lab SBC/compute-module options against concurrent graphics, local rules/generation, storage and peripheral workloads. Cloud remains authoritative for synchronized durable player state; exact local/offline acceptance policies remain open.

The native development baseline uses C/C++ toolchains for those targets. Compile and link firmware for real MCUs; use host adapters only for explicit behavioral testing. Active prototype development is authorized against the display contract below. Native builds must distinguish behavioral adapters from actual board drivers; physical timing, power and display readiness still require board evidence. See the [foundation development plan](../docs/builders/foundation-demo.md).

## Coupled constraints

The Lab must budget concurrent display, radio, storage, scanning and print preparation/transmission. The home station owns the printer mechanism, paper path, roll access and its power requirements. Verify exact logic levels separately from supply voltage; preserve reader sightline and service/programming access in the relevant device. Print conversion needs its own raster profile while preserving identity and scan quiet zones.

Probe sensor vents/windows must account for hand/body effects, enclosure, electronics and charging heat. Local valid measurements, optional phone context and generated fictional events remain distinct internally. Sensor capability limits future measured features; it does not limit fictional content to matching physical sensors. See [probe evidence](probe.md).

Both portables' cells, receiver coils, charging/load-sharing paths and caddy geometry form one coupled design. Avoid parallel chargers or backfeed between USB and wireless sources. Review cell-specific limits, receiver orientation, separation, retention, removal and sensor ventilation together. A shaped bay proves neither placement detection nor full charge.

Caddy charging is separate from data transfer, specimen handoff and gameplay scoring. Charge/full/fault/empty indications require actual supported signals; stale/missing reports are unknown. No screen, presence sensor or data channel is assumed.

## Validation gates

Firmware must expose domain-independent adapters for input, visible display readiness, sensing, storage, transport, print and power. Device profiles declare supported content, memory/raster limits and update compatibility; no profile can claim arbitrary future content support. Provisioning, firmware-update recovery and service diagnostics need a concrete design before deployment.

PCB and enclosure work are coupled: preserve connector and antenna clearances, sensor exposure, coil/cell separation, fasteners, tolerances and service access in a shared mechanical envelope. Board files, bills of materials and case drawings must identify compatible revisions. Actual interfaces and dimensions remain open; renders cannot substitute for schematics or fabrication drawings.

1. Rehearse pin/bus/address/voltage and physical envelopes before PCB/case commitments.
2. Measure native display/input, print/scan, storage/radio, energy and power transients under a concurrent workload.
3. Exercise portable interruption/restart and two-bay charging, removal/reseat/misalignment and safe fault behavior.
4. Recheck thermals, radio, sensing, charging and physical readability inside representative enclosures.
5. Produce repeatable assembly, programming, inspection and service instructions for a complete kit; CAD checks do not replace bench evidence.

The programme target is ten complete kits. US$750 is the maximum full-kit retail ceiling, aiming lower—not a BOM allowance or a verified selling price. Compare all four devices, cases, packaging/booklet, assembly/rework and support, without double-counting integrated controllers/chargers. Neither this target nor a concept render authorizes a parts selection.


## Display contract for active prototypes

Owner direction: **Lab color 1024×600 landscape; Probe portrait e-ink; Companion color 368×448 portrait**. These are distinct device profiles with separate composition and adapter behavior.

| Device | Selected requirement versus provisional hardware | Logical profile and implementation limit |
| --- | --- | --- |
| Lab | **Waveshare 7inch HDMI LCD (H), 1024×600 landscape color, selected for preliminary development.** Linux-class local execution accepted; exact host remains open. | **1024×600 landscape** native and logical profile. Lab screen implementation may proceed. HDMI video and USB capacitive touch require validation on the chosen Linux host. Earlier 400×240 and proposed 800×480 are superseded. Selection does not authorize purchase. |
| Probe | Portrait e-ink accepted. Waveshare 2.13-inch monochrome e-Paper HAT V4 is a **candidate**, not a final selected panel. | Candidate logical coordinates **122×250 portrait**, over its 250×122 controller raster; handle 90° rotation explicitly in panel adapter or renderer transform. Compose discrete monochrome frames; no smooth-animation or timed-reflex requirement. Change dimensions if another panel is selected. |
| Companion | **Waveshare ESP32-S3-Touch-AMOLED-1.8 selected for preliminary development:** 368×448 color AMOLED, ESP32-S3R8, 8 MB PSRAM, 16 MB flash. Exact PCB revision unconfirmed. | **368×448 portrait** with a rounded-corner safe region; its own scene/profile, never the Probe renderer flag. V1 SH8601/FT3168 and V2 CO5300/CST820 require matching BSP/drivers. Controls, battery, enclosure and production integration remain open. |
| Caddy | Small habitat display proposed; technology, resolution and autonomy unselected. | No raster/framebuffer contract yet. Status indicators require supported signals; missing information is unknown. |

Calculated native buffer examples: Lab RGB565 full frame is 1,228,800 bytes; XRGB8888 is 2,457,600 bytes; double buffering doubles either. Actual Linux scanout format/stride and renderer overhead remain to verify. Candidate Probe packed 1-bpp frame 4,000 bytes with byte-aligned rows, before old/new/partial-refresh buffers and driver reserve. Companion RGB565 329,728 bytes/full frame, 659,456 for two. RGB565 is an initial rendering/transfer assumption pending exact driver verification. Budget fonts, decoded assets/motion, stacks, journals, radio/TLS and internal DMA staging separately; PSRAM alone proves neither responsiveness nor runtime.

An RGB888 host image/scanline is a presentation adapter, not a panel-format buffer. Enforce each logical geometry, color policy and safe area, then convert through the selected display adapter. Controller orientation and memory stride remain distinct from player coordinates. Redesign Probe composition for portrait rather than stretching or rotating its landscape screenshot.

Each frame has an identity. Activation waits for that frame to be visibly ready; submitted pixels or SPI/DMA completion do not establish visibility. Consume gestures begun during wake/refresh through release and require fresh activation. E-ink exposes asynchronous busy/readiness/failure behavior; Companion supports bounded animation with a still fallback. Exact refresh scheduling and physical readiness require board evidence.

Existing evidence: [Companion documentation](https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-1.8) and [Probe manual](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual), inspected 26 September 2026. No new measurements implied. Pin revisions, mounting and drivers before hardware claims. The selected Lab [manufacturer product](https://www.waveshare.com/7inch-hdmi-lcd-h.htm) and [manual](https://files.waveshare.com/upload/5/58/7inch_HDMI_LCD_%28H%29_User_Manual.pdf) establish the 1024×600 IPS HDMI/USB-touch direction. Verify ordered revision, native mode/EDID, touch mapping, power and cable/enclosure clearance on the actual Linux host. Higher accepted HDMI input modes do not increase native pixels. No refresh, thermal or power performance has been measured; purchasing remains unauthorized.

## Lab controls — exploration in progress

Owner direction, 28 September 2026: explore a playful dedicated Lab instrument with separate navigation and meaningful manipulation controls. Directional arrows or other navigation controls, additional action buttons, and dedicated zoom/special-action knobs are permitted proposals. The existing simulator does not fix the final control count. A rotary knob need not perform menu navigation. A grid of colored, labeled workspace keys may provide direct access to Research, Library and other proposed destinations, separate from navigation and action controls. Explore different physical instrument identities before selecting a layout; neither minimum control count nor the current demo is the design objective.

The [four instrument concepts](../design/lab-controls/concepts.md) and [layout sheet](../design/lab-controls/four-concepts.png) compare genome exploration, specimen inspection, research and workspace switching. No winner is selected. Proposals must make the relationship between physical action and screen response visible, distinguish browsing/adjustment from costly commitment, and preserve sample/context on return. The prior minimalist one-knob/two-button recommendation is superseded as the design objective; its comparison image is retained as reference, not an approved configuration.

Final control count, grouping, functions, spacing and parts remain open. No live input-map change, purchase or enclosure/PCB commitment is implied. A future physical comparison must assess reach, tactile distinction, accidental inputs, screen/paper-path obstruction and base stability; no ergonomic measurements have been made.
