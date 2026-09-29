# Home and feature landings

The Home menu is the Lab's overview and entry point. Moving the directional
focus previews a feature's current state; Confirm enters that feature. The
overview itself never starts an expedition, buys research, opens an incubation
or performs care. Existing workspace buttons still provide direct access.

| Menu focus | Large overview | Confirm |
| --- | --- | --- |
| Home | Available resources, expedition/cargo, retained samples and discoveries, incubation and revealed residents | Remain on the overview |
| Explore | No expedition, gathering with elapsed active time/cargo, or completed haul awaiting return | Existing expedition view |
| Research | Retained sample count, selected sample and its discovered topics | Existing sample/research view |
| Incubator | Empty, incubating with real progress, or ready for deliberate opening | Existing incubator view |
| Habitat | Revealed residents and the selected resident's actual identity/care state, or an honest empty state | Existing habitat view |

Back returns from each feature to the corresponding focused Home row. Focus and
preview changes preserve game records, selected context and existing activities.
All input retains the visible-ready frame and fresh press/release contract.

The current V1 study resolves immediately after explicit confirmation/payment.
Research previews therefore show samples and discoveries, with no invented queue,
timer or background work. Incubation previews never expose an unrevealed individual.
Resource fractions remain real data and must not be rounded away for appearance.

## Visual derivation

[Approved refined C18](../game-art-proposals/35-vault-composition/18-c-refined.png)
and the [screen standard](../screen-design-standard.md) govern this implementation.
The Home composition uses a compact navigation rail and generous status area;
feature landings combine a relevant retained illustration with actionable status.
The lab-wide view presents several activities together rather than substituting
one decorative capsule for useful information.

Use fixed native insets and sibling footprints, stronger headline weight,
layered stepped electric-blue frames with dark edge/shadow separation, a quiet
warm selection treatment and the retained saturated resource/subject art.
Preserve readable native body type. The surrounding simulator case is outside
the native screen and remains the physical-control representation.

The overview uses unequal status regions according to information priority,
rather than four identical dashboard cards. Feature illustrations sit with their
readouts: cargo and active time, recorded topics, incubation progress, or known
resident information. A quiet dark warm tint and defined edge can supplement
selective warm corners; a solid bright action fill remains excluded. Licensed
native-size bold headings strengthen hierarchy without degrading body text.

This slice changes presentation and navigation, not the game rules or save
format. Actual rendered idle, active, ready and populated states are the review
surface; source-level style tokens alone do not establish visual fidelity.

Home keeps navigation guidance visible independently of the last action message.
Empty workspaces explain the relevant next step without promising unavailable
actions. Active or ready work receives the first status cue; static totals remain
quieter. Possible expedition finds are distinct from carried cargo. Sample topic
completion describes discovered knowledge, not a running research job.

## Native captures

These unscaled 1024×600 frames were produced by the native C renderer during the
real-time V1 journey at `59d6e0e9e8fc96973c45091883791679d162a368`.
They are host simulation evidence, not a Raspberry Pi board or physical display test.

- [Home with retained work and a resident](home.png)
- [Explore before departure](explore-idle.png) and [while gathering](explore-active.png)
- [Research with five discovered topics](research.png)
- [Incubation ready for deliberate opening](incubator-ready.png)
- [Habitat with a revealed resident](habitat.png)

The subsequent boundary correction narrows only a maximum-stock readout and
makes empty Incubator guidance depend on whether a researched sample is available;
these captured game states retain the same rendering.
