#include "game_rules.h"

#include "pip_genetics.h"
#include "save_bytes.h"

#include <stdio.h>
#include <string.h>

#define EXPEDITION_TICK_CAP 60u
#define GAME_STUDY_BITS ((1u << PIP_STUDY_COUNT) - 1u)

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
  case GAME_COMMAND_EXPEDITION_CONTINUE:
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
  case GAME_COMMAND_EXPEDITION_TRANSFER:
  case GAME_COMMAND_STOCK_NORMALIZE:
  case GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER:
  case GAME_COMMAND_EXPEDITION_FINISH:
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

int game_stock_normalized(const GameState *state) {
  return state && !state->legacy_supply_encoding &&
         state->data % GAME_SUPPLY_UNIT == 0 &&
         state->energy % GAME_SUPPLY_UNIT == 0 &&
         state->essence % GAME_SUPPLY_UNIT == 0 &&
         state->expedition_data % GAME_SUPPLY_UNIT == 0 &&
         state->expedition_energy % GAME_SUPPLY_UNIT == 0 &&
         state->expedition_essence % GAME_SUPPLY_UNIT == 0;
}

int game_supply_conversion_pending(const GameState *state) {
  return state && state->legacy_supply_encoding;
}

static int expedition_sample_ready(const GameState *state) {
  return state->expedition_id[0] &&
         state->expedition_elapsed >= GAME_EXPEDITION_SECONDS &&
         state->sample_count < GAME_MAX_SAMPLES;
}

int game_transfer_available(const GameState *state) {
  return state &&
         (state->expedition_data >= GAME_SUPPLY_UNIT ||
          state->expedition_energy >= GAME_SUPPLY_UNIT ||
          state->expedition_essence >= GAME_SUPPLY_UNIT ||
          expedition_sample_ready(state));
}

/* Version-two journal intents reserved this raw split policy. Keep it intact
 * until that intent is reconciled; only command twelve changes the encoding. */
static GameResult normalize_legacy_stock(GameState *state) {
  uint32_t *stock[] = {&state->data, &state->energy, &state->essence};
  uint32_t *carried[] = {&state->expedition_data, &state->expedition_energy,
                         &state->expedition_essence};
  uint32_t returned = 0;
  for (unsigned i = 0; i < 3; ++i)
    returned += *stock[i] % GAME_SUPPLY_UNIT;
  if (!returned)
    return GAME_DUPLICATE;
  if (cargo_total(state) + returned > GAME_CARGO_CAPACITY)
    return GAME_UNAVAILABLE;
  for (unsigned i = 0; i < 3; ++i) {
    uint32_t remainder = *stock[i] % GAME_SUPPLY_UNIT;
    *stock[i] -= remainder;
    *carried[i] += remainder;
  }
  return GAME_OK;
}

static GameResult convert_supply_encoding(GameState *state) {
  uint32_t *stock[] = {&state->data, &state->energy, &state->essence};
  uint32_t *carried[] = {&state->expedition_data, &state->expedition_energy,
                         &state->expedition_essence};
  if (!state->legacy_supply_encoding)
    return GAME_DUPLICATE;
  for (unsigned i = 0; i < 3; ++i) {
    uint32_t stock_remainder = *stock[i] % GAME_SUPPLY_UNIT;
    uint32_t carried_remainder = *carried[i] % GAME_SUPPLY_UNIT;
    *stock[i] -= stock_remainder;
    *carried[i] -= carried_remainder;
    /* Historical partial work becomes time credit, never an earned item.
     * Credits may exceed one interval; later qualified attempts consume them. */
    state->gather_progress_ms[i] =
        (stock_remainder + carried_remainder) * GAME_GATHER_ATTEMPT_MS /
        GAME_SUPPLY_UNIT;
  }
  state->gather_random_state = GAME_GATHER_INITIAL_RANDOM_STATE;
  state->gather_attempt_count = 0;
  state->gather_last_attempted_mask = 0;
  state->gather_last_awarded_mask = 0;
  state->legacy_supply_encoding = 0;
  return GAME_OK;
}

uint32_t game_gather_remaining_ms(const GameState *state) {
  uint32_t remaining = GAME_GATHER_ATTEMPT_MS;
  if (!state || state->legacy_supply_encoding)
    return remaining;
  for (unsigned i = 0; i < 3; ++i) {
    uint32_t class_remaining =
        state->gather_progress_ms[i] >= GAME_GATHER_ATTEMPT_MS
            ? 0
            : GAME_GATHER_ATTEMPT_MS - state->gather_progress_ms[i];
    if (class_remaining < remaining)
      remaining = class_remaining;
  }
  return remaining;
}

unsigned game_gather_due_mask(const GameState *state) {
  unsigned mask = 0;
  uint32_t remaining = game_gather_remaining_ms(state);
  if (!state || state->legacy_supply_encoding)
    return 0;
  for (unsigned i = 0; i < 3; ++i) {
    uint32_t class_remaining =
        state->gather_progress_ms[i] >= GAME_GATHER_ATTEMPT_MS
            ? 0
            : GAME_GATHER_ATTEMPT_MS - state->gather_progress_ms[i];
    if (class_remaining == remaining)
      mask |= 1u << i;
  }
  return mask;
}

unsigned game_gather_required_slots(const GameState *state) {
  unsigned count = 0;
  if (!state || state->legacy_supply_encoding)
    return 0;
  for (unsigned i = 0; i < 3; ++i)
    if (state->gather_progress_ms[i] + 1000u >= GAME_GATHER_ATTEMPT_MS)
      ++count;
  return count;
}

int game_gather_capacity_blocked(const GameState *state) {
  return !state || state->legacy_supply_encoding ||
         cargo_total(state) >= GAME_CARGO_CAPACITY ||
         game_gather_required_slots(state) * GAME_SUPPLY_UNIT >
             GAME_CARGO_CAPACITY - cargo_total(state);
}

static int make_id(char *destination, size_t capacity, const char *prefix,
                   uint32_t identity) {
  int count = snprintf(destination, capacity, "BEE-%s-%05u", prefix, identity);
  return count >= 0 && (size_t)count < capacity;
}

static GameResult record_expedition_sample(GameState *state) {
  if (expedition_sample_ready(state)) {
    GameSample *sample = &state->samples[state->sample_count];
    memset(sample, 0, sizeof(*sample));
    if (!make_id(sample->id, sizeof(sample->id), "S", state->next_identity++))
      return GAME_INVALID;
    strcpy(sample->origin_expedition_id, state->expedition_id);
    sample->origin_expedition_kind = (uint8_t)state->expedition_kind;
    sample->supported_candidates = PIP_SAMPLE_CANDIDATE_MASK;
    ++state->sample_count;
  }
  return GAME_OK;
}

static GameResult transfer_expedition(GameState *state) {
  uint32_t *stock[] = {&state->data, &state->energy, &state->essence};
  uint32_t *carried[] = {&state->expedition_data, &state->expedition_energy,
                         &state->expedition_essence};
  uint32_t complete[3];
  if (!cargo_total(state) && !expedition_sample_ready(state))
    return GAME_UNAVAILABLE;
  for (unsigned i = 0; i < 3; ++i) {
    complete[i] = *carried[i] - *carried[i] % GAME_SUPPLY_UNIT;
    *carried[i] -= complete[i];
  }
  /* Returning old Lab fractions here avoids changing an immutable sealed
   * snapshot before its explicit acceptance. All changes share one save. */
  GameResult normalized = normalize_legacy_stock(state);
  if (normalized != GAME_OK && normalized != GAME_DUPLICATE)
    return normalized;
  for (unsigned i = 0; i < 3; ++i)
    if (!add_stock(stock[i], complete[i]))
      return GAME_UNAVAILABLE;
  GameResult sample_result = record_expedition_sample(state);
  if (sample_result != GAME_OK)
    return sample_result;
  state->expedition_active = 0;
  if (state->expedition_elapsed >= GAME_EXPEDITION_SECONDS) {
    state->expedition_elapsed = 0;
    state->expedition_id[0] = '\0';
  }
  return GAME_OK;
}

static GameResult transfer_whole_expedition(GameState *state) {
  int legacy_haul = state->legacy_supply_encoding && cargo_total(state);
  if (state->legacy_supply_encoding) {
    GameResult converted = convert_supply_encoding(state);
    if (converted != GAME_OK)
      return converted;
  }
  if (!legacy_haul && !game_transfer_available(state))
    return GAME_UNAVAILABLE;
  if (!add_stock(&state->data, state->expedition_data) ||
      !add_stock(&state->energy, state->expedition_energy) ||
      !add_stock(&state->essence, state->expedition_essence))
    return GAME_UNAVAILABLE;
  GameResult sample_result = record_expedition_sample(state);
  if (sample_result != GAME_OK)
    return sample_result;
  state->expedition_data = 0;
  state->expedition_energy = 0;
  state->expedition_essence = 0;
  state->expedition_active = 0;
  if (state->expedition_elapsed >= GAME_EXPEDITION_SECONDS) {
    state->expedition_elapsed = 0;
    state->expedition_id[0] = '\0';
  }
  return GAME_OK;
}

static uint32_t next_gather_random(GameState *state) {
  uint32_t value = state->gather_random_state;
  value ^= value << 13;
  value ^= value >> 17;
  value ^= value << 5;
  state->gather_random_state = value;
  return value;
}

static int apply_expedition_tick(GameState *state, uint32_t now) {
  uint32_t elapsed;
  uint32_t index;
  if (!state->expedition_active || state->legacy_supply_encoding)
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
    uint32_t *carried[] = {&state->expedition_data, &state->expedition_energy,
                           &state->expedition_essence};
    unsigned required = game_gather_required_slots(state);
    unsigned attempted = 0;
    unsigned awarded = 0;
    /* Reserve worst-case room before consuming time or any chance draw. This
     * lets capacity pauses resume without discarding a success or rerolling. */
    if (game_gather_capacity_blocked(state) ||
        state->gather_attempt_count > UINT64_MAX - required)
      return index ? GAME_OK : GAME_UNAVAILABLE;
    for (unsigned i = 0; i < 3; ++i) {
      state->gather_progress_ms[i] += 1000u;
      if (state->gather_progress_ms[i] >= GAME_GATHER_ATTEMPT_MS) {
        state->gather_progress_ms[i] -= GAME_GATHER_ATTEMPT_MS;
        attempted |= 1u << i;
        ++state->gather_attempt_count;
        /* Provisional V1 fixture: each class independently succeeds on three
         * of four outcomes. Stable class order and saved PRNG prevent rerolls. */
        if (next_gather_random(state) % 4u < 3u) {
          *carried[i] += GAME_SUPPLY_UNIT;
          awarded |= 1u << i;
        }
      }
    }
    if (attempted) {
      state->gather_last_attempted_mask = (uint8_t)attempted;
      state->gather_last_awarded_mask = (uint8_t)awarded;
    }
    ++state->expedition_elapsed;
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
        state->legacy_supply_encoding ||
        !state->runtime_anchors_ready)
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
  case GAME_COMMAND_EXPEDITION_CONTINUE:
    if (state->expedition_active || !state->expedition_id[0] ||
        state->expedition_elapsed >= GAME_EXPEDITION_SECONDS ||
        state->legacy_supply_encoding ||
        !state->runtime_anchors_ready)
      return GAME_UNAVAILABLE;
    if (command->data.monotonic_seconds < state->expedition_last_tick)
      return GAME_INVALID;
    state->expedition_active = 1;
    state->expedition_last_tick = command->data.monotonic_seconds;
    return GAME_OK;
  case GAME_COMMAND_EXPEDITION_TRANSFER:
    if (!state->legacy_supply_encoding)
      return GAME_UNAVAILABLE;
    return transfer_expedition(state);
  case GAME_COMMAND_STOCK_NORMALIZE:
    return convert_supply_encoding(state);
  case GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER:
    return transfer_whole_expedition(state);
  case GAME_COMMAND_EXPEDITION_FINISH:
    if (state->legacy_supply_encoding || state->expedition_active ||
        !state->expedition_id[0] ||
        state->expedition_elapsed < GAME_EXPEDITION_SECONDS ||
        game_transfer_available(state))
      return GAME_UNAVAILABLE;
    state->expedition_id[0] = '\0';
    state->expedition_elapsed = 0;
    return GAME_OK;
  case GAME_COMMAND_EXPEDITION_OFFLOAD: {
    uint32_t total = cargo_total(state);
    if (!state->legacy_supply_encoding || !state->expedition_id[0] || !total)
      return GAME_UNAVAILABLE;
    if (!add_stock(&state->data, state->expedition_data) ||
        !add_stock(&state->energy, state->expedition_energy) ||
        !add_stock(&state->essence, state->expedition_essence))
      return GAME_UNAVAILABLE;
    GameResult sample_result = record_expedition_sample(state);
    if (sample_result != GAME_OK)
      return sample_result;
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
    if (!state->legacy_supply_encoding &&
        command->data.discard.quantity % GAME_SUPPLY_UNIT != 0)
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
    if (!game_stock_normalized(state) || sample->incubated ||
        state->data < study->cost_data ||
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
    if (!game_stock_normalized(state) || state->data < 500u ||
        state->energy < 500u || state->essence < 500u)
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
