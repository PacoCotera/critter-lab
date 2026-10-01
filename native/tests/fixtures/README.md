# Frozen save compatibility fixtures

`runtime-v2.save` is actual isolated native runtime output from source revision
04ea5a3. Its original bytes are retained unchanged; it contains an active
expedition, committed operation fingerprints and saved gathering/chance state.
It contains no player or private operational data.

`authored-v1.save` and `authored-v2.save` are synthetic historical-ABI fixtures,
built with fixed offsets and the original checksum independently of the current
C structs. They contain partial and completed five-study records, a revealed
legacy founder, an operation fingerprint and exact stock. V1 retains fractional
legacy encoding. V2 includes nonzero tail padding in its original checksum.
These are authored compatibility checks, not output from historical gameplay.

The frozen V1 payload/file lengths are 5728/5752 bytes; V2 is 5760/5784 bytes,
including the five V2 tail-padding bytes. The manifest records each original
hash and provenance. The generator only rebuilds the two authored fixtures.


`field-v1.save` is frozen isolated native output from source864eca5, delivered
through public merge26b4a875. It holds an active version1 field at Camp, no active
source and no earned cargo/sample. Its exact legacy coordinates/terrain/paths are
retained through load, navigation, trace, accepted history and another save.
This is runtime geometry compatibility evidence, not a human playtest or an
expedition balance fixture. It contains no player/private operational data.
