#ifndef PIP_GENETICS_H
#define PIP_GENETICS_H

#include "game_state.h"

#include <stdint.h>

#define PIP_STUDY_COUNT 5u
#define PIP_SAMPLE_CANDIDATE_MASK 0x03u
/* Five variable-locus study facts for this pinned Pip content package only. */
#define PIP_REQUIRED_VARIABLE_FACTS_MASK 0x1fu
#define PIP_REQUIRED_FACTS_MASK PIP_REQUIRED_VARIABLE_FACTS_MASK
#define PIP_CONTENT_VERSION "pip-proof-v1"
#define PIP_RULES_VERSION "pip-rules-v1"
#define PIP_ART_VERSION "pip-playtest-art-v1"

typedef struct {
  const char *locus_id;
  const char *title;
  const char *allele_a;
  const char *allele_b;
  const char *finding;
  uint32_t fact_mask;
  uint16_t cost_data;
  uint16_t cost_energy;
  uint16_t cost_essence;
} PipStudy;

const PipStudy *pip_studies(void);
const PipStudy *pip_study(unsigned index);
int pip_genome_valid(const PipGenome *genome);
int pip_genome_for_sample(unsigned sample_ordinal, PipGenome *genome);
int pip_expression_valid(const PipGenome *genome,
                         const PipExpression *expression);
void pip_express(const PipGenome *genome, PipExpression *expression);
const char *pip_art_id(const PipGenome *genome);

#endif
