#define _POSIX_C_SOURCE 200809L
#include "game_state.h"

#include "pip_genetics.h"
#include "save_bytes.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#define SAVE_MAGIC "BEECHOV1"
#define GAME_BALANCE_MAX_STOCK 1000000u

typedef struct {
  char magic[8];
  uint32_t version;
  uint32_t payload_size;
  uint32_t checksum;
  GameState state;
} SavedGame;

static uint32_t checksum_bytes(const unsigned char *bytes, size_t length) {
  uint32_t value = 2166136261u;
  size_t index;
  for (index = 0; index < length; ++index) {
    value ^= bytes[index];
    value *= 16777619u;
  }
  return value;
}

static int text_valid(const char *text, size_t capacity, int may_be_empty) {
  const char *end = memchr(text, '\0', capacity);
  return end && (may_be_empty || end != text);
}

void game_state_init(GameState *state) {
  memset(state, 0, sizeof(*state));
  state->version = GAME_STATE_VERSION;
  strcpy(state->balance_version, GAME_BALANCE_VERSION);
  state->next_identity = 1;
  state->incubation_sample = 0xffu;
  state->incubation_individual = 0xffu;
}

int game_state_valid(const GameState *state) {
  uint64_t cargo;
  unsigned index;
  if (!state || state->version != GAME_STATE_VERSION ||
      strcmp(state->balance_version, GAME_BALANCE_VERSION) != 0 ||
      state->revision != state->last_operation_sequence ||
      state->operation_cursor >= GAME_OPERATION_SLOTS ||
      state->next_identity == 0 || state->expedition_active > 1 ||
      state->incubation_active > 1 || state->incubation_ready > 1 ||
      state->sample_count > GAME_MAX_SAMPLES ||
      state->individual_count > GAME_MAX_INDIVIDUALS ||
      state->expedition_kind > GAME_EXPEDITION_RESONANCE ||
      state->expedition_elapsed > GAME_EXPEDITION_SECONDS ||
      state->incubation_elapsed > GAME_INCUBATION_SECONDS ||
      state->habitat >= GAME_HABITAT_COUNT || state->reserved != 0 ||
      state->runtime_commit_uncertain > 1)
    return 0;
  cargo = (uint64_t)state->expedition_data + state->expedition_energy +
          state->expedition_essence;
  if (cargo > GAME_CARGO_CAPACITY || state->data > GAME_BALANCE_MAX_STOCK ||
      state->energy > GAME_BALANCE_MAX_STOCK ||
      state->essence > GAME_BALANCE_MAX_STOCK ||
      (state->expedition_active &&
       (state->expedition_id[0] == '\0' ||
        state->expedition_elapsed >= GAME_EXPEDITION_SECONDS)) ||
      (cargo != 0 && state->expedition_id[0] == '\0') ||
      (cargo == 0 && !state->expedition_active &&
       state->expedition_id[0] != '\0') ||
      state->runtime_anchors_ready > 1 ||
      !text_valid(state->expedition_id, sizeof(state->expedition_id), 1))
    return 0;
  if (state->incubation_active) {
    if (state->incubation_sample >= state->sample_count ||
        state->incubation_individual >= state->individual_count ||
        !state->samples[state->incubation_sample].incubated ||
        (state->incubation_ready !=
         (state->incubation_elapsed >= GAME_INCUBATION_SECONDS)))
      return 0;
  } else if (state->incubation_sample != 0xffu ||
             state->incubation_individual != 0xffu ||
             state->incubation_ready || state->incubation_elapsed != 0) {
    return 0;
  }

  for (index = 0; index < state->sample_count; ++index) {
    const GameSample *sample = &state->samples[index];
    unsigned earlier;
    if (!text_valid(sample->id, sizeof(sample->id), 0) ||
        !text_valid(sample->origin_expedition_id,
                    sizeof(sample->origin_expedition_id), 0) ||
        (sample->decoded_studies & ~((1u << PIP_STUDY_COUNT) - 1u)) ||
        (sample->decoded_facts & ~PIP_REQUIRED_FACTS_MASK) ||
        sample->supported_candidates != PIP_SAMPLE_CANDIDATE_MASK ||
        sample->origin_expedition_kind > GAME_EXPEDITION_RESONANCE ||
        sample->incubated > 1 ||
        ((sample->decoded_studies == ((1u << PIP_STUDY_COUNT) - 1u)) !=
         (sample->decoded_facts == PIP_REQUIRED_FACTS_MASK)))
      return 0;
    for (earlier = 0; earlier < index; ++earlier)
      if (strcmp(sample->id, state->samples[earlier].id) == 0) return 0;
  }
  for (index = state->sample_count; index < GAME_MAX_SAMPLES; ++index)
    if (state->samples[index].id[0] != '\0') return 0;

  for (index = 0; index < state->individual_count; ++index) {
    const GameIndividual *individual = &state->individuals[index];
    unsigned earlier;
    int source_found = 0;
    unsigned sample_index;
    if (!text_valid(individual->id, sizeof(individual->id), 0) ||
        !text_valid(individual->source_sample_id,
                    sizeof(individual->source_sample_id), 0) ||
        !text_valid(individual->origin_kind, sizeof(individual->origin_kind), 0) ||
        !text_valid(individual->art_id, sizeof(individual->art_id), 0) ||
        !text_valid(individual->art_version, sizeof(individual->art_version), 0) ||
        strcmp(individual->art_version, PIP_ART_VERSION) != 0 ||
        individual->origin_founder != 1 || individual->revealed > 1 ||
        individual->art_pending != 0 ||
        !pip_genome_valid(&individual->genome) ||
        strcmp(individual->art_id, pip_art_id(&individual->genome)) != 0 ||
        individual->habitat >= GAME_HABITAT_COUNT ||
        individual->care_visits > GAME_MAX_CARE_VISITS ||
        !pip_expression_valid(&individual->genome, &individual->expression))
      return 0;
    for (sample_index = 0; sample_index < state->sample_count; ++sample_index)
      if (strcmp(individual->source_sample_id,
                 state->samples[sample_index].id) == 0 &&
          state->samples[sample_index].incubated)
        source_found = 1;
    if (!source_found) return 0;
    for (earlier = 0; earlier < index; ++earlier)
      if (strcmp(individual->id, state->individuals[earlier].id) == 0)
        return 0;
  }
  for (index = state->individual_count; index < GAME_MAX_INDIVIDUALS; ++index)
    if (state->individuals[index].id[0] != '\0') return 0;

  for (index = 0; index < GAME_OPERATION_SLOTS; ++index) {
    const GameOperation *operation = &state->operations[index];
    unsigned earlier;
    if (operation->sequence == 0) {
      if (operation->id[0] != '\0' || operation->fingerprint != 0) return 0;
      continue;
    }
    if (!text_valid(operation->id, sizeof(operation->id), 0) ||
        operation->sequence > state->last_operation_sequence)
      return 0;
    for (earlier = 0; earlier < index; ++earlier)
      if (state->operations[earlier].sequence == operation->sequence ||
          strcmp(state->operations[earlier].id, operation->id) == 0)
        return 0;
  }
  return 1;
}

int game_state_save(const char *path, const GameState *state) {
  SavedGame saved;
  int lock_fd;
  int result;
  if (!path || !game_state_valid(state)) return -1;
  memset(&saved, 0, sizeof(saved));
  memcpy(saved.magic, SAVE_MAGIC, sizeof(saved.magic));
  saved.version = GAME_STATE_VERSION;
  saved.payload_size = (uint32_t)sizeof(saved.state);
  saved.state = *state;
  saved.state.runtime_anchors_ready = 0;
  saved.state.runtime_commit_uncertain = 0;
  saved.state.expedition_last_tick = 0;
  saved.state.incubation_last_tick = 0;
  saved.checksum = checksum_bytes((const unsigned char *)&saved.state,
                                 sizeof(saved.state));
  lock_fd = save_bytes_lock(path);
  if (lock_fd < 0) return -1;
  result = save_bytes_write_status(path, &saved, sizeof(saved));
  (void)flock(lock_fd, LOCK_UN);
  (void)close(lock_fd);
  return result;
}

int game_state_load(const char *path, GameState *state) {
  SavedGame saved;
  FILE *file;
  size_t read_count;
  int extra;
  if (!path || !state) return -1;
  file = fopen(path, "rb");
  if (!file) return errno == ENOENT ? 1 : -1;
  read_count = fread(&saved, 1, sizeof(saved), file);
  extra = fgetc(file);
  if (fclose(file) || read_count != sizeof(saved) || extra != EOF ||
      memcmp(saved.magic, SAVE_MAGIC, sizeof(saved.magic)) != 0 ||
      saved.version != GAME_STATE_VERSION ||
      saved.payload_size != sizeof(saved.state) ||
      saved.checksum != checksum_bytes((const unsigned char *)&saved.state,
                                       sizeof(saved.state)) ||
      !game_state_valid(&saved.state))
    return -1;
  *state = saved.state;
  state->runtime_anchors_ready = 0;
  state->runtime_commit_uncertain = 0;
  state->expedition_last_tick = 0;
  state->incubation_last_tick = 0;
  return 0;
}
