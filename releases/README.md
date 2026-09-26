# Versions and compatibility

There is no production release or frozen 1.0 specification yet. Git commits identify exact source snapshots.

## What changes independently

| Versioned material | Why it matters |
| --- | --- |
| Product specifications | Approved direction can precede implementation |
| App, service and firmware source | Identifies the code actually running |
| Saved-record and protocol formats | Determines what consumers can read and exchange |
| Genetics, expression and content rules | Determines how inherited information is interpreted |
| Appearance and animation assets | Preserves an individual's finished identity |
| Hardware profiles | Identifies supported devices and physical capabilities |

A deployable release must state supported combinations, required migrations and behavior for unsupported data. Breaking changes need migration instructions and compatibility examples. Preserve saved genomes, expression context, versions and finished assets; updating content must not silently rewrite individuals or acquired history.

Design labels (**accepted**, **proposed**, **open**) are distinct from implementation evidence (**unimplemented**, **host experiment**, **integrated**, **physically validated**). The [specification map](../specs/README.md) explains those labels; [build coverage](../BUILD.md) lists current evidence.

Future release notes should identify the source commit, changed components, compatible data/content/hardware versions, build/install instructions, migration or recovery steps and known limitations. This describes required release information, not an existing release system.
