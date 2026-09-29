# Devices and physical interfaces

Status: intended functions and engineering constraints. No selected production parts, board layout, enclosure dimensions, thermal margin or battery runtime is established.

| Device | Required role | Open physical/firmware choices |
| --- | --- | --- |
| Lab / Console | Connected research workbench, local configuration, cached archive, color pixel display, thermal print and QR reading; play without a Probe | Selected development display: Waveshare 7inch HDMI LCD (H), 1024×600 landscape color; Linux host, driver integration, controls, storage, power and print/scan remain open |
| Field Probe | Standalone evidence and resource gathering, player-attributed expeditions, retained cargo and recoverable offload | Sensor suite, cadence, capacity, battery, exact portrait e-ink panel and transport; no required phone |
| Companion | Shared player use, saved-individual display, training/evolution/bonding and temporary offline activity | Number of carried critters, supported actions, handover, display/controls and power |
| Caddy | Wireless charging for both portables | Receiver/transmitter solution, separate supply, mechanical alignment, status signal and indicators |

The [physical-experience principle](experience.md#physical-experience-is-the-product) defines why these are distinct objects: simple e-ink collection, responsive companionship, an extensible Lab workbench and a physical charging home. It guides engineering exploration without selecting production modules.

Warm beige shells, charcoal structure and restrained orange accents inform the product family. Concept proportions and earlier renders do not freeze controls, ports, materials or manufacture. No physical slots are implied by virtual Lab chips.

## Development targets

Owner direction: a more capable Linux-class Lab, an nRF52840 Probe and an ESP32-S3 Companion. The Lab display and Companion board/display below are selected for preliminary development; the Probe panel remains a candidate. Sensor sets, battery configurations and production parts remain open. Compare Lab SBC/compute-module options against concurrent graphics, local rules/generation, storage and peripheral workloads. Cloud remains authoritative for synchronized durable player state; exact local/offline acceptance policies remain open.

The native development baseline uses C/C++ toolchains for those targets. Compile and link firmware for real MCUs; use host adapters only for explicit behavioral testing. Active prototype development is authorized against the display contract below. Native builds must distinguish behavioral adapters from actual board drivers; physical timing, power and display readiness still require board evidence. See the [foundation development plan](../docs/builders/foundation-demo.md).

## Coupled constraints

The Console must budget concurrent display, radio, storage, scanning and dense printing. Verify exact logic levels separately from supply voltage; preserve accessible paper path, roll change, reader sightline and service/programming access. Print conversion needs its own raster profile while preserving identity and scan quiet zones.

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
| Caddy | No display selected or required. | No raster/framebuffer. Indicators use supported electrical status; missing information is unknown. |

Calculated native buffer examples: Lab RGB565 full frame is 1,228,800 bytes; XRGB8888 is 2,457,600 bytes; double buffering doubles either. Actual Linux scanout format/stride and renderer overhead remain to verify. Candidate Probe packed 1-bpp frame 4,000 bytes with byte-aligned rows, before old/new/partial-refresh buffers and driver reserve. Companion RGB565 329,728 bytes/full frame, 659,456 for two. RGB565 is an initial rendering/transfer assumption pending exact driver verification. Budget fonts, decoded assets/motion, stacks, journals, radio/TLS and internal DMA staging separately; PSRAM alone proves neither responsiveness nor runtime.

An RGB888 host image/scanline is a presentation adapter, not a panel-format buffer. Enforce each logical geometry, color policy and safe area, then convert through the selected display adapter. Controller orientation and memory stride remain distinct from player coordinates. Redesign Probe composition for portrait rather than stretching or rotating its landscape screenshot.

Each frame has an identity. Activation waits for that frame to be visibly ready; submitted pixels or SPI/DMA completion do not establish visibility. Consume gestures begun during wake/refresh through release and require fresh activation. E-ink exposes asynchronous busy/readiness/failure behavior; Companion supports bounded animation with a still fallback. Exact refresh scheduling and physical readiness require board evidence.

Existing evidence: [Companion documentation](https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-1.8) and [Probe manual](https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual), inspected 26 September 2026. No new measurements implied. Pin revisions, mounting and drivers before hardware claims. The selected Lab [manufacturer product](https://www.waveshare.com/7inch-hdmi-lcd-h.htm) and [manual](https://files.waveshare.com/upload/5/58/7inch_HDMI_LCD_%28H%29_User_Manual.pdf) establish the 1024×600 IPS HDMI/USB-touch direction. Verify ordered revision, native mode/EDID, touch mapping, power and cable/enclosure clearance on the actual Linux host. Higher accepted HDMI input modes do not increase native pixels. No refresh, thermal or power performance has been measured; purchasing remains unauthorized.

## Lab controls — exploration in progress

Owner direction, 28 September 2026: explore a playful dedicated Lab instrument with separate navigation and meaningful manipulation controls. Directional arrows or other navigation controls, additional action buttons, and dedicated zoom/special-action knobs are permitted proposals. The existing simulator does not fix the final control count. A rotary knob need not perform menu navigation. A grid of colored, labeled workspace keys may provide direct access to Research, Library and other proposed destinations, separate from navigation and action controls. Explore different physical instrument identities before selecting a layout; neither minimum control count nor the current demo is the design objective.

The [four instrument concepts](../design/lab-controls/concepts.md) and [layout sheet](../design/lab-controls/four-concepts.png) compare genome exploration, specimen inspection, research and workspace switching. No winner is selected. Proposals must make the relationship between physical action and screen response visible, distinguish browsing/adjustment from costly commitment, and preserve sample/context on return. The prior minimalist one-knob/two-button recommendation is superseded as the design objective; its comparison image is retained as reference, not an approved configuration.

Final control count, grouping, functions, spacing and parts remain open. No live input-map change, purchase or enclosure/PCB commitment is implied. A future physical comparison must assess reach, tactile distinction, accidental inputs, screen/paper-path obstruction and base stability; no ergonomic measurements have been made.


