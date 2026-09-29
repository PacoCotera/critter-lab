#include "pip_genetics.h"

#include <string.h>

/* Provisional balance v1. Costs, expedition yields and durations are fixture
 * values for the playable slice; they are not canonical game balance. */
static const PipStudy STUDIES[PIP_STUDY_COUNT] = {
  {.locus_id="form.crown", .title="Crown form", .allele_a="C", .allele_b="c",
   .finding="A soft crown frill frames the head.", .fact_mask=1u << 0,
   .cost_data=500, .cost_energy=0, .cost_essence=0},
  {.locus_id="appearance.rings", .title="Eye rings", .allele_a="R", .allele_b="r",
   .finding="Pale rings surround amber eyes.", .fact_mask=1u << 1,
   .cost_data=0, .cost_energy=500, .cost_essence=0},
  {.locus_id="appearance.markings", .title="Body markings", .allele_a="P", .allele_b="p",
   .finding="Plain coats can carry pale markings. Both forms are possible.", .fact_mask=1u << 2,
   .cost_data=0, .cost_energy=0, .cost_essence=500},
  {.locus_id="movement.drive", .title="Movement", .allele_a="M", .allele_b="m",
   .finding="Six legs carry Pip in short, quick bursts.", .fact_mask=1u << 3,
   .cost_data=400, .cost_energy=400, .cost_essence=0},
  {.locus_id="movement.efficiency", .title="Energy use", .allele_a="E", .allele_b="e",
   .finding="Efficient movement leaves more energy for exploring.", .fact_mask=1u << 4,
   .cost_data=0, .cost_energy=400, .cost_essence=400},
};

const PipStudy *pip_studies(void) { return STUDIES; }

const PipStudy *pip_study(unsigned index) {
  return index < PIP_STUDY_COUNT ? &STUDIES[index] : 0;
}

int pip_genome_valid(const PipGenome *genome) {
  static const char EXPECTED[GAME_GENETIC_LOCI][2] = {
      {'C', 'c'}, {'R', 'r'}, {0, 0}, {'M', 'm'}, {'E', 'e'}};
  unsigned locus;
  if (!genome || strcmp(genome->class_id, "critter:pip") != 0 ||
      strcmp(genome->content_version, PIP_CONTENT_VERSION) != 0 ||
      strcmp(genome->rules_version, PIP_RULES_VERSION) != 0)
    return 0;
  for (locus = 0; locus < GAME_GENETIC_LOCI; ++locus) {
    if (locus == 2) {
      int carried = genome->loci[locus][0] == 'P' &&
                    genome->loci[locus][1] == 'p';
      int expressed = genome->loci[locus][0] == 'p' &&
                      genome->loci[locus][1] == 'p';
      if (!carried && !expressed) return 0;
      continue;
    }
    if (genome->loci[locus][0] != EXPECTED[locus][0] ||
        genome->loci[locus][1] != EXPECTED[locus][1])
      return 0;
  }
  return 1;
}

int pip_genome_for_sample(unsigned candidate, PipGenome *genome) {
  static const char COMMON[GAME_GENETIC_LOCI][2] = {
      {'C', 'c'}, {'R', 'r'}, {'P', 'p'}, {'M', 'm'}, {'E', 'e'}};
  if (!genome || candidate > 1) return -1;
  memset(genome, 0, sizeof(*genome));
  memcpy(genome->loci, COMMON, sizeof(COMMON));
  if (candidate == 1) {
    genome->loci[2][0] = 'p';
    genome->loci[2][1] = 'p';
  }
  strcpy(genome->class_id, "critter:pip");
  strcpy(genome->content_version, PIP_CONTENT_VERSION);
  strcpy(genome->rules_version, PIP_RULES_VERSION);
  return 0;
}

void pip_express(const PipGenome *genome, PipExpression *expression) {
  memset(expression, 0, sizeof(*expression));
  expression->crown = genome->loci[0][0] == 'C' || genome->loci[0][1] == 'C';
  expression->eye_rings = genome->loci[1][0] == 'R' || genome->loci[1][1] == 'R';
  expression->pale_markings = genome->loci[2][0] == 'p' && genome->loci[2][1] == 'p';
  expression->burst_movement = genome->loci[3][0] == 'M' || genome->loci[3][1] == 'M';
  expression->efficient_movement = genome->loci[4][0] == 'E' || genome->loci[4][1] == 'E';
}

const char *pip_art_id(const PipGenome *genome) {
  if (!pip_genome_valid(genome)) return 0;
  return genome->loci[2][0] == 'p' && genome->loci[2][1] == 'p'
             ? "design/v1-pip/pip-marked.png"
             : "design/v1-pip/pip-carried.png";
}

int pip_expression_valid(const PipGenome *genome,
                         const PipExpression *expression) {
  PipExpression resolved;
  if (!pip_genome_valid(genome) || !expression) return 0;
  pip_express(genome, &resolved);
  return memcmp(&resolved, expression, sizeof(resolved)) == 0;
}
