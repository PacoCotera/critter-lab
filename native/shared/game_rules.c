#include "game_rules.h"

#include "pip_genetics.h"
#include "save_bytes.h"

#include <stdio.h>
#include <string.h>

#define EXPEDITION_TICK_CAP 60u
#define GAME_STUDY_BITS ((1u << PIP_STUDY_COUNT) - 1u)

/* Internal stock units are 1/1000 of a pack. Yields are per active second. */
static const uint32_t YIELD_PER_ACTIVE_SECOND[3] = {22u, 22u, 22u};

static uint64_t hash_byte(uint64_t value, unsigned char byte) {
  return (value ^ byte) * 1099511628211ULL;
}

static uint64_t hash_u32(uint64_t value, uint32_t input) {
  unsigned index;
  for (index = 0; index < 4; ++index) {
    value = hash_byte(value, (unsigned char)(input & 0xffu));
    input >>= 8;
  }
  return value;
}

static uint64_t command_fingerprint(const GameCommand *command) {
  uint64_t value = 14695981039346656037ULL;
  const unsigned char *text = (const unsigned char *)command->operation_id;
  while (*text)
    value = hash_byte(value, *text++);
  value = hash_u32(value, (uint32_t)command->type);
  switch (command->type) {
  case GAME_COMMAND_EXPEDITION_START:
    value = hash_u32(value, (uint32_t)command->data.expedition.kind);
    value = hash_u32(value, command->data.expedition.monotonic_seconds);
    break;
  case GAME_COMMAND_EXPEDITION_TICK:
  case GAME_COMMAND_INCUBATION_TICK:
    value = hash_u32(value, command->data.monotonic_seconds);
    break;
  case GAME_COMMAND_EXPEDITION_DISCARD:
    value = hash_u32(value, (uint32_t)command->data.discard.resource);
    value = hash_u32(value, command->data.discard.quantity);
    value = hash_u32(value, command->data.discard.confirm);
    break;
  case GAME_COMMAND_STUDY:
    value = hash_u32(value, command->data.study.sample);
    value = hash_u32(value, command->data.study.study);
    break;
  case GAME_COMMAND_INCUBATION_START:
    value = hash_u32(value, command->data.creation.sample);
    value = hash_u32(value, command->data.creation.preference);
    value = hash_u32(value, command->data.creation.monotonic_seconds);
    break;
  case GAME_COMMAND_HABITAT_VISIT:
    value = hash_u32(value, command->data.habitat.individual);
    value = hash_u32(value, command->data.habitat.habitat);
    break;
  case GAME_COMMAND_CARE_VISIT:
    value = hash_u32(value, command->data.individual);
    break;
  case GAME_COMMAND_EXPEDITION_OFFLOAD:
  case GAME_COMMAND_INCUBATION_OPEN:
    break;
  default:
    break;
  }
  return value;
}

static int add_stock(uint32_t *stock, uint32_t amount) {
  if (*stock > 1000000u - amount)
    return 0;
  *stock += amount;
  return 1;
}

static unsigned cargo_total(const GameState *state) {
  return state->expedition_data + state->expedition_energy +
         state->expedition_essence;
}

static int make_id(char *destination, size_t capacity, const char *prefix,
                   uint32_t identity) {
  int count = snprintf(destination, capacity, "BEE-%s-%05u", prefix, identity);
  return count >= 0 && (size_t)count < capacity;
}

static int apply_expedition_tick(GameState *state, uint32_t now) {
  uint32_t elapsed;
  uint32_t index;
  if (!state->expedition_active)
    return GAME_UNAVAILABLE;
  if (!state->runtime_anchors_ready)
    return GAME_INVALID;
  if (now < state->expedition_last_tick)
    return GAME_INVALID;
  elapsed = now - state->expedition_last_tick;
  if (elapsed > EXPEDITION_TICK_CAP)
    elapsed = EXPEDITION_TICK_CAP;
  state->expedition_last_tick = now;
  for (index = 0;
       index < elapsed && state->expedition_elapsed < GAME_EXPEDITION_SECONDS;
       ++index) {
    uint32_t after = cargo_total(state) + YIELD_PER_ACTIVE_SECOND[0] +
                     YIELD_PER_ACTIVE_SECOND[1] + YIELD_PER_ACTIVE_SECOND[2];
    if (after > GAME_CARGO_CAPACITY)
      return GAME_UNAVAILABLE;
    state->expedition_data += YIELD_PER_ACTIVE_SECOND[0];
    state->expedition_energy += YIELD_PER_ACTIVE_SECOND[1];
    state->expedition_essence += YIELD_PER_ACTIVE_SECOND[2];
    ++state->expedition_elapsed;
    if (after >= GAME_CARGO_CAPACITY)
      break;
  }
  if (state->expedition_elapsed >= GAME_EXPEDITION_SECONDS)
    state->expedition_active = 0;
  return GAME_OK;
}

static int apply_incubation_tick(GameState *state, uint32_t now) {
  uint32_t elapsed;
  if (!state->incubation_active || state->incubation_ready)
    return GAME_UNAVAILABLE;
  if (!state->runtime_anchors_ready)
    return GAME_INVALID;
  if (now < state->incubation_last_tick)
    return GAME_INVALID;
  elapsed = now - state->incubation_last_tick;
  if (elapsed > EXPEDITION_TICK_CAP)
    elapsed = EXPEDITION_TICK_CAP;
  state->incubation_last_tick = now;
  if (elapsed > GAME_INCUBATION_SECONDS - state->incubation_elapsed)
    elapsed = GAME_INCUBATION_SECONDS - state->incubation_elapsed;
  state->incubation_elapsed += elapsed;
  state->incubation_ready =
      state->incubation_elapsed >= GAME_INCUBATION_SECONDS;
  return GAME_OK;
}

static GameResult apply_domain_command(GameState *state,
                                       const GameCommand *command) {
  switch (command->type) {
  case GAME_COMMAND_EXPEDITION_START: {
    if (command->data.expedition.kind > GAME_EXPEDITION_RESONANCE ||
        state->expedition_active || state->expedition_id[0] ||
        cargo_total(state) || !state->runtime_anchors_ready)
      return GAME_UNAVAILABLE;
    state->expedition_active = 1;
    state->expedition_kind = (uint32_t)command->data.expedition.kind;
    state->expedition_elapsed = 0;
    state->expedition_last_tick = command->data.expedition.monotonic_seconds;
    if (!make_id(state->expedition_id, sizeof(state->expedition_id), "E",
                 state->next_identity++))
      return GAME_INVALID;
    return GAME_OK;
  }
  case GAME_COMMAND_EXPEDITION_TICK:
    return (GameResult)apply_expedition_tick(state,
                                             command->data.monotonic_seconds);
  case GAME_COMMAND_EXPEDITION_OFFLOAD: {
    GameSample *sample;
    uint32_t total = cargo_total(state);
    int award_sample = state->expedition_elapsed >= GAME_EXPEDITION_SECONDS &&
                       state->sample_count < GAME_MAX_SAMPLES;
    if (!state->expedition_id[0] || !total)
      return GAME_UNAVAILABLE;
    if (!add_stock(&state->data, state->expedition_data) ||
        !add_stock(&state->energy, state->expedition_energy) ||
        !add_stock(&state->essence, state->expedition_essence))
      return GAME_UNAVAILABLE;
    if (award_sample) {
      sample = &state->samples[state->sample_count];
      memset(sample, 0, sizeof(*sample));
      if (!make_id(sample->id, sizeof(sample->id), "S", state->next_identity++))
        return GAME_INVALID;
      strcpy(sample->origin_expedition_id, state->expedition_id);
      sample->origin_expedition_kind = (uint8_t)state->expedition_kind;
      sample->supported_candidates = PIP_SAMPLE_CANDIDATE_MASK;
      ++state->sample_count;
    }
    state->expedition_data = 0;
    state->expedition_energy = 0;
    state->expedition_essence = 0;
    state->expedition_elapsed = 0;
    state->expedition_active = 0;
    state->expedition_id[0] = '\0';
    return GAME_OK;
  }
  case GAME_COMMAND_EXPEDITION_DISCARD: {
    uint32_t *resource = 0;
    if (!state->expedition_id[0] || cargo_total(state) == 0 ||
        command->data.discard.quantity == 0 || !command->data.discard.confirm)
      return GAME_UNAVAILABLE;
    if (command->data.discard.resource == GAME_RESOURCE_DATA)
      resource = &state->expedition_data;
    else if (command->data.discard.resource == GAME_RESOURCE_ENERGY)
      resource = &state->expedition_energy;
    else if (command->data.discard.resource == GAME_RESOURCE_ESSENCE)
      resource = &state->expedition_essence;
    if (!resource || command->data.discard.quantity > *resource)
      return GAME_INVALID;
    *resource -= command->data.discard.quantity;
    return GAME_OK;
  }
  case GAME_COMMAND_STUDY: {
    const PipStudy *study = pip_study(command->data.study.study);
    GameSample *sample;
    unsigned index = command->data.study.sample;
    uint8_t bit;
    if (!study || index >= state->sample_count)
      return GAME_INVALID;
    sample = &state->samples[index];
    bit = (uint8_t)(1u << command->data.study.study);
    if (sample->decoded_studies & bit)
      return GAME_DUPLICATE;
    if (sample->incubated || state->data < study->cost_data ||
        state->energy < study->cost_energy ||
        state->essence < study->cost_essence)
      return GAME_UNAVAILABLE;
    state->data -= study->cost_data;
    state->energy -= study->cost_energy;
    state->essence -= study->cost_essence;
    sample->decoded_studies |= bit;
    sample->decoded_facts |= study->fact_mask;
    return GAME_OK;
  }
  case GAME_COMMAND_INCUBATION_START: {
    unsigned sample_index = command->data.creation.sample;
    unsigned preference = command->data.creation.preference;
    GameSample *sample;
    GameIndividual *individual;
    if (sample_index >= state->sample_count || preference > 1 ||
        state->incubation_active ||
        state->individual_count >= GAME_MAX_INDIVIDUALS)
      return GAME_INVALID;
    sample = &state->samples[sample_index];
    if (sample->decoded_studies != GAME_STUDY_BITS || sample->incubated)
      return GAME_UNAVAILABLE;
    if (state->data < 500u || state->energy < 500u || state->essence < 500u)
      return GAME_UNAVAILABLE;
    individual = &state->individuals[state->individual_count];
    memset(individual, 0, sizeof(*individual));
    if (!make_id(individual->id, sizeof(individual->id), "P",
                 state->next_identity++))
      return GAME_INVALID;
    strcpy(individual->source_sample_id, sample->id);
    strcpy(individual->origin_kind, "parentless-founder");
    individual->origin_founder = 1;
    if (pip_genome_for_sample(preference, &individual->genome) != 0)
      return GAME_INVALID;
    strcpy(individual->art_id, pip_art_id(&individual->genome));
    strcpy(individual->art_version, PIP_ART_VERSION);
    pip_express(&individual->genome, &individual->expression);
    state->data -= 500u;
    state->energy -= 500u;
    state->essence -= 500u;
    ++state->individual_count;
    sample->incubated = 1;
    state->incubation_active = 1;
    state->incubation_ready = 0;
    state->incubation_sample = (uint8_t)sample_index;
    state->incubation_individual = (uint8_t)(state->individual_count - 1u);
    state->incubation_choice = (uint8_t)preference;
    state->incubation_elapsed = 0;
    state->incubation_last_tick = command->data.creation.monotonic_seconds;
    return GAME_OK;
  }
  case GAME_COMMAND_INCUBATION_TICK:
    return (GameResult)apply_incubation_tick(state,
                                             command->data.monotonic_seconds);
  case GAME_COMMAND_INCUBATION_OPEN: {
    GameIndividual *individual;
    if (!state->incubation_active || !state->incubation_ready ||
        state->incubation_individual >= state->individual_count)
      return GAME_UNAVAILABLE;
    individual = &state->individuals[state->incubation_individual];
    if (individual->revealed)
      return GAME_DUPLICATE;
    individual->revealed = 1;
    state->incubation_active = 0;
    state->incubation_ready = 0;
    state->incubation_sample = 0xffu;
    state->incubation_individual = 0xffu;
    state->incubation_choice = 0;
    state->incubation_elapsed = 0;
    return GAME_OK;
  }
  case GAME_COMMAND_HABITAT_VISIT: {
    unsigned index = command->data.habitat.individual;
    if (index >= state->individual_count ||
        command->data.habitat.habitat >= GAME_HABITAT_COUNT)
      return GAME_INVALID;
    if (!state->individuals[index].revealed)
      return GAME_UNAVAILABLE;
    if (state->individuals[index].habitat == command->data.habitat.habitat)
      return GAME_DUPLICATE;
    state->individuals[index].habitat = (uint8_t)command->data.habitat.habitat;
    state->habitat = (uint8_t)command->data.habitat.habitat;
    return GAME_OK;
  }
  case GAME_COMMAND_CARE_VISIT: {
    unsigned index = command->data.individual;
    if (index >= state->individual_count)
      return GAME_INVALID;
    if (!state->individuals[index].revealed)
      return GAME_UNAVAILABLE;
    if (state->individuals[index].care_visits >= GAME_MAX_CARE_VISITS)
      return GAME_DUPLICATE;
    ++state->individuals[index].care_visits;
    return GAME_OK;
  }
  default:
    return GAME_INVALID;
  }
}

void game_rules_resume_runtime(GameState *state, uint32_t monotonic_seconds) {
  if (!state)
    return;
  state->runtime_anchors_ready = 1;
  if (state->expedition_active)
    state->expedition_last_tick = monotonic_seconds;
  if (state->incubation_active && !state->incubation_ready)
    state->incubation_last_tick = monotonic_seconds;
}

GameResult game_apply(const char *path, GameState *state,
                      const GameCommand *command) {
  GameState candidate;
  uint64_t fingerprint;
  size_t operation_length;
  unsigned index;
  GameResult result;
  int save_result;
  if (!path || !state || !command || !command->operation_id ||
      !command->operation_id[0] || command->sequence == 0 ||
      !game_state_valid(state))
    return GAME_INVALID;
  operation_length = strlen(command->operation_id);
  if (operation_length >= sizeof(state->operations[0].id))
    return GAME_INVALID;
  fingerprint = command_fingerprint(command);
  if (command->sequence <= state->last_operation_sequence) {
    for (index = 0; index < GAME_OPERATION_SLOTS; ++index) {
      const GameOperation *previous = &state->operations[index];
      if (previous->sequence == command->sequence) {
        return strcmp(previous->id, command->operation_id) == 0 &&
                       previous->fingerprint == fingerprint
                   ? GAME_DUPLICATE
                   : GAME_CONFLICT;
      }
    }
    return GAME_DUPLICATE;
  }
  if (state->runtime_commit_uncertain)
    return GAME_COMMITTED_UNCERTAIN;
  if (command->sequence != state->last_operation_sequence + 1u)
    return GAME_INVALID;
  for (index = 0; index < GAME_OPERATION_SLOTS; ++index)
    if (state->operations[index].sequence &&
        strcmp(state->operations[index].id, command->operation_id) == 0)
      return GAME_CONFLICT;

  candidate = *state;
  result = apply_domain_command(&candidate, command);
  if (result != GAME_OK)
    return result;
  candidate.last_operation_sequence = command->sequence;
  candidate.revision = command->sequence;
  strcpy(candidate.operations[candidate.operation_cursor].id,
         command->operation_id);
  candidate.operations[candidate.operation_cursor].sequence = command->sequence;
  candidate.operations[candidate.operation_cursor].fingerprint = fingerprint;
  candidate.operation_cursor =
      (candidate.operation_cursor + 1u) % GAME_OPERATION_SLOTS;
  if (!game_state_valid(&candidate))
    return GAME_INVALID;
  save_result = game_state_save(path, &candidate);
  if (save_result == SAVE_BYTES_NOT_COMMITTED)
    return GAME_STORAGE;
  *state = candidate;
  if (save_result == SAVE_BYTES_COMMITTED_DURABILITY_UNCERTAIN) {
    state->runtime_commit_uncertain = 1;
    return GAME_COMMITTED_UNCERTAIN;
  }
  return GAME_OK;
}
