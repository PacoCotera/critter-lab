# Critter Lab roadmap

## Design before implementation

Review the whole Companion → Lab → resident → Caddy journey before dependent
implementation. Architecture and game design are being revisited under the
[design decision boundary](README.md#design-before-the-next-implementation).
Existing software, concepts and authoring results are evidence, not approval of
the new baseline. Preserve accepted unchanged direction and original references.

The [connected review packet](design/probe-bench-review.md#read-this-checkpoint-first)
now records the confirmed loop and [game tenets](specs/gameplay.md#governing-game-tenets).
The [whole-game subsystem map](specs/gameplay.md#whole-game-subsystem-map) includes
breeding as core, collection/nurturing, environment, observability, genome
production, inventory, social play, printing and cloud mechanics. Refine their
dependencies and device contributions before choosing the next implementation.
The earlier encounter-to-research slice remains a candidate, not a full-game plan
or an approved implementation assignment. C18 is concept art to refine.

## Milestones and feature tracking

GitHub issues carry task state and acceptance; this page is the outcome map.
The later milestones are gated planning horizons, with no asserted dates or
automatic authority to begin implementation.

| Milestone | Feature and completion gate |
| --- | --- |
| [M0: design baseline](https://github.com/PacoCotera/critter-lab/milestone/1) | [Connected architecture, game, UI/UX and art decisions](https://github.com/PacoCotera/critter-lab/issues/105); design review selects direction and the first bounded slice |
| [M1: connected play](https://github.com/PacoCotera/critter-lab/milestone/2) | [Playability and discovery](https://github.com/PacoCotera/critter-lab/issues/44), [setup/recovery](https://github.com/PacoCotera/critter-lab/issues/108); one approved whole journey with actual-use evidence |
| [M2: creatures and individuality](https://github.com/PacoCotera/critter-lab/milestone/3) | [Genomic construction and art](https://github.com/PacoCotera/critter-lab/issues/60), [identity/lineage/continuity](https://github.com/PacoCotera/critter-lab/issues/106), [compatible breeding](https://github.com/PacoCotera/critter-lab/issues/112); only approved contracts and accepted assets |
| [M3: physical validation](https://github.com/PacoCotera/critter-lab/milestone/4) | [Combined Companion, Lab and Caddy kit](https://github.com/PacoCotera/critter-lab/issues/107); sourced choices and measured bench evidence before commitments |
| [M4: pilot readiness](https://github.com/PacoCotera/critter-lab/milestone/5) | [Assembly and support](https://github.com/PacoCotera/critter-lab/issues/109); selected pilot after physical validation |

M0 resolves the exact sequence and scope. Retained prototypes and closed delivery
PRs are evidence; they do not close these outstanding product acceptance gates.

## Player outcome

Complete the authorized Polished Core V1 journey: Companion gathers and returns; Lab accepts once, retains sample discoveries and supports deliberate creation; incubation reveals the same saved resident on Lab and Companion; Dock shows accepted or clearly cached world facts. Functional migration does not approve fun, art or hardware readiness.

## Current delivery

Native graphics-framework coverage is complete for current connected device pages. The obsolete standalone acquisition renderer is retired while domain/input/save regression remains. [Architecture](specs/architecture.md#current-migration-coverage-and-target-evidence) owns target distinctions; [retirement evidence](docs/evidence/lvgl-route-retirement/README.md) records checks. Companion entry/haul repair and Habitat population browsing are delivered; [status](STATUS.md) records actual acceptance. [Deployment notes](CHANGELOG.md) must match the displayed sandbox revision. Native gameplay increments use the existing Git, CI and native-frame delivery gate
with fresh sandbox saves. Hosting-only changes preserve the native release and
saved world, with separate hosting metadata.

The hosted [guided authoring workspace](prototype/generator-workbench/evidence/guided-authoring/README.md)
and [actual Google rendering/retention](prototype/generator-workbench/evidence/api-rendering/README.md)
are delivered bounded experiments. They do not complete full-genome construction,
accepted creature art, animation or configured game creation; [status](STATUS.md)
records current evidence and gaps.

## Design review and pending outcomes

1. **Connected design review:** bring the whole player journey, worked discovery-to-resident example, physical inputs and visible responses, system authority/data flow, offline/recovery boundaries and consequential open choices together. Design review selects changed game and architecture direction before dependent implementation. Use existing [gameplay](specs/gameplay.md), [architecture](specs/architecture.md) and [experience](specs/experience.md) documents rather than another competing specification.
2. **Research discovery — implementation paused:** follow the delivered Habitat preview/commit grammar: directions update passive previews; Confirm enters a task or deliberately commits an action; Back returns. No navigation may spend resources, transfer cargo or record a visit. Existing sample destinations/aggregate Overview and focused Library findings preview immediately; explanations/status belong in main. Replace the checklist experience with specimen-specific clues, unanswered questions, directed studies and retained comparisons. Use the [comparison trial](design/research-and-creation.md#next-comparison-trial), [genetic constraints](specs/genetics.md) and actual playtest. Automatic analysis remains accepted; no guessed-answer quiz is selected. [Issue60](https://github.com/PacoCotera/critter-lab/issues/60) holds current integration and acceptance status.
3. **Exploration depth:** open traversable terrain, useful generated variation, distinguishable expedition opportunities and one retained local discovery/event. The [exploration study](design/expedition-map-study/README.md#gathering-review-exploration-and-source-decisions) is a design input. Current native saved geometry is implemented, but road restrictions and common finite source rules remain. Do not claim device generation or differentiated expedition behavior from a paper study or UI change.
4. **Visual craft:** replace the rejected incubator with an actual Gemini-sourced protected biological chamber; preserve unknown contents until Open. Continue C18/Gemini/HiBit composition, margins, hierarchy and meaningful visuals through actual native exports. Existing assets and useful unfinished references remain preserved.

Design review ends in a concrete design decision recorded in the relevant public specification. Authorized software increments end in a small coherent PR, focused independent review, relevant actual-use evidence and delivery; native gameplay changes retain physical-control proof. Do not hold a reviewed increment for unrelated art or whole-milestone consolidation. Current defects and broader review findings remain in [issue44](https://github.com/PacoCotera/critter-lab/issues/44) and the [three-device audit](design/three-device-playability-audit/README.md).

## Boundaries

No console-only acquisition, new infrastructure, purchases, real-radio/hardware commitment, capture/training/ecology or production cloud deployment. Existing bounded Google/OpenAI authoring adapters do not select a production game service or authorize broader provider work. Continue the selected physical controls and existing standard graphics framework. Preserve whole-unit resources, sample identity, discovery authority and explicit complete-genome creation. Hardware development follows satisfactory software play; a mobile fallback remains allowed if the device experience does not justify building the kit.
