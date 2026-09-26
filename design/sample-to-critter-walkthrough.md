# From field sample to living critter

**Public review walkthrough — proposed details, not implemented behavior.** Critter Lab connects exploration, research, creation and an ongoing relationship with an individual. This small example follows one player, Mara, from an ordinary sample to a saved critter. Working labels, experiment responses, ingredients and the illustrated genetic subset are **PROPOSED**; they are not canonical creature designs or a limit on future diversity.

The governing directions are in [gameplay](../specs/gameplay.md) and [genetics](../specs/genetics.md): research supported possibilities, fully unlock the genome, choose a complete configuration before creation, and deliberately open the saved individual.

## Bring something home

Mara takes a shared Probe on an expedition assigned to her profile. Environmental observations contribute to a fictional sample and collection progress. The Probe measures conditions, not real genes; resource awards are separate game rules. Her sample, supplies and knowledge remain hers when somebody else uses the same Probe or Lab. A console-only investigation can provide equivalent opportunities with laboratory provenance.

At the Lab, Mara selects **Sample: Ripple**. Its recognizable bitmap stays stable as knowledge grows. The example below is a conceptual region map, not a production encoding, nucleotide sequence or rule that each pixel equals a gene.

| Bitmap region | On arrival | After the relevant study |
| --- | --- | --- |
| ▧ A ▧ | Required information unresolved | Supported crown variants |
| ▧ B ▧ | Required information unresolved | Eye-ring variants |
| ▧ C ▧ | Required information unresolved | Pale-marking variants |

Annotations explain the regions; research does not repaint the sample into a new identity. A known carried variant has a readable label, never the same symbol as unknown information. These region labels explain this example; they do not establish navigation groups or replace the five genetic information layers.

## Choose an investigation

**PROPOSED:** Ripple offers **Material study: examine the outline** and **Variation study: examine the markings**. Mara can choose either first. Each card identifies its question, displayed reagent cost and unresolved region; there are no sliders or correct-answer quizzes.

Material study reveals the supported crown and smooth-head alternatives in region A. Variation study explains regions B/C: eye rings express; a pale-marking variant is carried without expressing in this example. Different sample profiles would have different clues, responses and requirements under the same research rules. These fictional tests do not claim that physical materials prove particular alleles.

Mara lacks the reagent for Variation study. **PROPOSED shortage behavior:** that experiment waits; the completed outline finding, other eligible work and encyclopedia remain available. The Lab suggests finding suitable supplies or preparing them through console crafting. A clue describes ingredient properties and relevant conditions. Mara can experiment without first unlocking recipe permission. She records the useful preparation in her personal encyclopedia; exact quantities and crafting-failure recovery are outside this walkthrough.

Running a study commits its displayed resources once. Its saved result remains inspectable; revisiting does not charge again, reroll the finding or award duplicate supplies. No automatic critter appears.

## Know the genome, then commit

Every required region is now readable. **PROPOSED complete configurations for this declared miniature model**, under pinned expression rules and no pale activation:

| Choice | Full modeled genotype | Expressed appearance | Carried but unexpressed |
| --- | --- | --- | --- |
| Crowned | `Cc / Rr / Pp` | Crown, eye rings | Pale-marking variant |
| Smooth | `cc / Rr / Pp` | Smooth head, eye rings | Pale-marking variant |

Mara chooses Crowned. Preview shows the complete genotype, expressed/carried distinction, body-plan reference, expression context and creation costs. It does not secretly generate unmodeled abilities or behavioral genes. This is a narrow complete fixture, not a claim that the full product genome consists of cosmetic switches.

**PROPOSED:** explicit Create commits the chosen configuration, sample and displayed creation supplies together. The result is a new individual with its own identity and parentless laboratory origin. A sample is provenance, not a parent. Social research references cannot substitute as donor material.

## Meet—and keep meeting—the same individual

Phenotype describes expressed appearance and capabilities. Artwork depicts that phenotype. Behavior uses supported capabilities, inherited tendencies and changing lifetime state; an animation is a presentation of behavior, not a new genetic fact. This cosmetic fixture establishes no hidden temperament or practical power.

READY waits for Mara to choose OPEN. The saved portrait and a proposed greeting presentation bring the known features together. Surprise comes from meeting the character, not undisclosed genetic changes. Its silhouette, markings and identity remain recognizable in inspection, on paper and on the Companion. Companion training and bonding belong to Mara’s individual; sharing the device does not transfer ownership or merge another player’s progress. Learned history does not automatically become inherited genetics.

## If the connection drops

Keep the selected sample and last verified state visible. Before submission, an unavailable connection leaves the action uncommitted. After an uncertain submission, check that same operation before retrying; do not spend again or create another individual. Report pending versus confirmed honestly. Cloud-unconfirmed activity is not promised recoverable from another device. A missing portrait preserves the saved individual and allows fetching its existing art again.

## Review the system boundary

The paired [system contract](../specs/sample-to-critter-contract.md) owns the proposed transaction and failure rules, and the consolidated open decisions. Review that boundary before extending the fixture with functional genetics, a larger economy or living behavior.
