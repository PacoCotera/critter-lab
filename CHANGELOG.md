# Sandbox deployment changelog

Major changes in the [playable sandbox](https://critterlab.basicberry.com/sandbox/). Times use CDMX. Entries record live deployments; source-only work belongs in its PR until activation.

## 2026-10-03 [15:10] — Programmatic pet-rendering API transport

[PR100](https://github.com/PacoCotera/critter-lab/pull/100) · hosting source `57db139` · native game remains `720c1e6`

- The workbench can submit its retained 512px source image and short brief to a selected image-provider API, keeping returned candidates linked to exact source, prompt and provider/job metadata.
- Google is configured first. OpenAI has an adapter and remains unavailable until configured. Requests require an explicit Render action; provider polling and recovery do not generate again.
- Durable server storage keeps completed results recoverable after browser loss. The original API presentation still needs the owner-requested editable prompt, creature gallery and guided authoring overhaul; this release establishes transport, not completed usability or accepted pet art.

Independent genomic/pixel-source and technical inspection, unchanged automatic CI37153443058 and exact clean pushed-source VM builds passed. Existing website, simulator, native binary and saves retained; no reset, new tests or increased testing scope. No actual API-generated image had been requested at activation.

## 2026-10-03 [14:19] — Optional genome-derived innate profile

[PR99](https://github.com/PacoCotera/critter-lab/pull/99) · hosting source `50d5a7a` · native game remains `720c1e6`

- The authoring bench now carries114 ordered pairs and six drafts. Three provisional Cognition contributors resolve an optional static profile: presence, exploration tendency and reference-cue threshold.
- OFF retains all copies and witnesses; changing the profile does not change source geometry, material or the short art brief. Old111-pair records keep their original versions.
- This is partial eleven-layer authoring data, without live behavior or improved creature silhouettes. Four domains still lack executable contracts.

Independent source/domain review, unchanged automatic CI and normal exact clean pushed-source VM build passed. Existing website, native binary and saves retained; no reset or increased testing scope.

## 2026-10-03 [13:30] — Critter Lab website and future product vision

[PR98](https://github.com/PacoCotera/critter-lab/pull/98) · hosting source `1a9cde0` · native game remains `720c1e6`

- Restored Critter Lab branding and the intended Companion → Lab → individual/family → Caddy/paper story. Planned features remain part of that vision.
- Added genome authoring, source-derived short art briefs and source-bound retained proposals, with clear separation from game residents.
- Paired the retained original ecosystem concept with current native Research evidence; corrected historical gallery labels and preserved full family/Caddy framing.

[Website assessment and actual publication evidence](website/evidence/vision-refresh/README.md) retain desktop/phone captures, image provenance, independent review and live source/native receipts. Existing host, routes, native binary and saved-world configuration retained; no reset, new tests, discretionary test runs, CI expansion or native build.
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
