# Devices and physical interfaces

Status: intended functions and engineering constraints. No selected production parts, board layout, enclosure dimensions, thermal margin or battery runtime is established.

| Device | Required role | Open physical/firmware choices |
| --- | --- | --- |
| Lab / Console | Connected research workbench, local configuration, cached archive, pixel display, thermal print and QR reading; play without a Probe | Display/controller/driver, controls, storage, power and print/scan interfaces; OLED, monochrome/six-color e-ink and LCD exploration do not select a module |
| Field Probe | Standalone evidence and resource gathering, player-attributed expeditions, retained cargo and recoverable offload | Sensor suite, cadence, capacity, battery, screen and transport; no required phone |
| Companion | Shared player use, saved-individual display, training/evolution/bonding and temporary offline activity | Number of carried critters, supported actions, handover, display/controls and power |
| Caddy | Wireless charging for both portables | Receiver/transmitter solution, separate supply, mechanical alignment, status signal and indicators |

The [physical-experience principle](experience.md#physical-experience-is-the-product) defines why these are distinct objects: simple e-ink collection, responsive companionship, an extensible Lab workbench and a physical charging home. It guides engineering exploration without selecting production modules.

Warm beige shells, charcoal structure and restrained orange accents inform the product family. Concept proportions and earlier renders do not freeze controls, ports, materials or manufacture. No physical slots are implied by virtual Lab chips.

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
