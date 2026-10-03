# Sandbox deployment changelog

Major changes in the [playable sandbox](https://critterlab.basicberry.com/sandbox/). Times use CDMX. Entries record live deployments; source-only work belongs in its PR until activation.

## 2026-10-03 [12:47] — Website and genome workbench on the project domain

[PR97](https://github.com/PacoCotera/critter-lab/pull/97) · hosting source `ad192ef` · native game remains `720c1e6`

- **Website:** the project introduction is at [the domain root](https://critterlab.basicberry.com/), with direct links to the workbench and simulator.
- **Genome workbench:** [the hosted bench](https://critterlab.basicberry.com/genome/) exposes current authoring, genome-derived source, exact short Gemini prompt, browser retention and replay. It remains an incomplete eleven-layer developer prototype.
- **Simulator:** the existing Lab, Companion and Dock moved to `/sandbox/`. Native binary, release and save configuration are unchanged; no reset was invoked. Hosting revision metadata is separate from native release metadata.

[Actual public-origin evidence](docs/evidence/hosted-platform/README.md) retains screenshots, source, prompt and reopened record. Localhost browser records require explicit export/import to the HTTPS origin. The owner restored the name Critter Lab and assigned the website content/concept refresh separately after this hosting activation.

## 2026-10-01 [14:49] — Research inquiry and retained findings

[PR67](https://github.com/PacoCotera/critter-lab/pull/67) · release `720c1e6`

- **Research:** highlighting a sample or Library finding updates its exact preview immediately. The left rail holds destinations; clues, questions, resource costs, Start and findings appear in the main workpiece.
- **Discovery references:** known coat evidence shows permitted original references without revealing an unknown whole form. Complete comparisons preserve the original portraits; linked movement and effort findings remain distinct.
- **Controls:** browsing and known inspection are free. Fresh Start commits the shown cost; Back restores the selected inquiry. Existing rules and saved findings remain intact.
- **Sandbox saves:** this activation starts a fresh game across all three devices.

Research enjoyment and wider sample variety remain open gameplay work.

## 2026-10-01 [13:50] — Habitat population and resident browsing

[PR65](https://github.com/PacoCotera/critter-lab/pull/65) · release `5e16e75`

- **Habitat:** the overview shows the population together. The Residents gallery displays up to eight revealed critters, with each saved identity, form and source available while browsing.
- **Controls:** spatial directions change the selected resident and its preview immediately. Confirm opens that resident's activity; only the separate care action records a visit. Back returns to the same resident.
- **Sandbox saves:** this activation starts a fresh game across all three devices.

## 2026-10-01 [13:03] — Companion entry and haul handoff

[PR64](https://github.com/PacoCotera/critter-lab/pull/64) · release `ee8ae71`

- **Companion:** the home screen presents Probe, Cargo and Companions as distinct destinations. Expedition choices follow their vertical Up/Down layout.
- **Cargo:** Send requires one deliberate selection; Lab acceptance remains separate. Acceptance clears current Companion cargo and the Lab reception source, while received records retain the historical haul.
- **Sandbox saves:** this activation starts a fresh game across all three devices.

## 2026-10-01 [12:04] — Deployment notes in the sandbox

[PR63](https://github.com/PacoCotera/critter-lab/pull/63) · release `669d0bd`

- **Sandbox:** a Deployment changelog link beside the release details opens these public notes. Entries group the biggest changes by device/domain and distinguish delivered work from open improvements.
- **Sandbox saves:** this activation starts a fresh game across all three devices.

## 2026-10-01 [11:50] — Graphics framework cleanup

[PR62](https://github.com/PacoCotera/critter-lab/pull/62) · release `292a305`

- **Graphics:** retired the old direct-pixel Lab rendering paths. Current device pages stay within LVGL; unsupported historical test pages fail explicitly.
- **Gameplay:** existing controls, quantities and saved-state behavior are preserved. This activation starts fresh sandbox saves.

## 2026-10-01 [11:25] — Device UI and gathering update

[PR61](https://github.com/PacoCotera/critter-lab/pull/61) · release `3fdda1d`

- **Companion:** finite whole-unit gathering is immediate; field preparation waits are removed.
- **Simulator:** Companion is larger, with Back beside Confirm.
- **UI:** repeated navigation instructions are removed; focus shows selection. Native text and shared screen frames use LVGL.
- **Lab:** research, creation, incubation and resident screens use the same graphics framework while preserving costs, saved findings and critter identity.
- **Dock:** four-level grayscale replaces the old black/white-only output.
- **Sandbox:** activation starts fresh Lab, Companion and Dock saves.

**Still open:** the incubator canister is rejected artwork awaiting replacement. Final art quality, discovery depth and hardware behavior remain under development.
