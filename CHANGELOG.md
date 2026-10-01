# Sandbox deployment changelog

Major changes in the [playable sandbox](https://critterlab.basicberry.com/). Times use CDMX. Entries record live deployments; source-only work belongs in its PR until activation.

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
