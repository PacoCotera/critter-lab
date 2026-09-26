# Native expedition replacement

Experimental replacement for the existing expedition-to-research prototype. This is a headless native model and persistence layer; no replacement renderer, host clock worker or browser preview is connected yet. The legacy live sandbox is unchanged.

## State and disclosure

Progress, resource awards, observation state, sealed sample and Lab research are separate facts. The Probe receives a restricted typed projection with no research result, genetic region or sample-content inference. Lab research consumes a supply explicitly and preserves its finding for free revisits.

The named simulation profile lasts 120,000 accepted milliseconds, awards supplies once at 30,000 and 90,000 ms, exposes a neutral fictional event at 45,000 ms and seals the sample at completion. These are replaceable engineering fixtures, not final game balance or sensor measurements.

## Clock and retries

The future host adapter supplies a service epoch and monotonic offset. A new epoch establishes a baseline without crediting stopped-service time. Starting anchors the clock; Check, inspection and browser viewing do not earn progress.

Clock cursor and user-command receipt are independent. User retry identity is command name, expected revision and operation ID. Host clock metadata is excluded so the same user operation can reconcile after service restart; a replay returns the prior effect before applying any new clock observation.

## Files and validation

- model: state validation, transitions, native actions and restricted device views.
- store: explicit versioned serialization, reusing shared atomic byte storage.
- main: headless command/status/tick adapter.

Build with CMake in a separate build directory. Run native/tests/test_replacement.py against the resulting executable. Tests cover threshold crossings, repeated ticks, restart, uncertain writes, command retries, disclosure, research spending and invalid-save preservation. Host-worker scheduling, native rendering and browser integration still require their own checks when implemented.

Use a separate replacement save. Do not point this binary at the legacy live save, reinterpret old fixture values as measured time, or silently migrate/reset a player's progress.
