#include "expedition.h"
#include "pip_genetics.h"
#include <stdio.h>
#include <string.h>

#define FIELD_ATTEMPTS 12u
static unsigned cell(unsigned x, unsigned y) { return y * 20u + x;
}
unsigned game_field_source_resource(unsigned source) {
  static const unsigned resources[GAME_FIELD_SOURCES] = {0, 1, 2, 2, 0, 1};
  return source < GAME_FIELD_SOURCES ? resources[source] : GAME_FIELD_NONE;
}
unsigned game_field_source_site(unsigned source) {
  static const unsigned sites[GAME_FIELD_SOURCES] = {0, 0, 0, 1, 2, 3};
  return source < GAME_FIELD_SOURCES ? sites[source] : GAME_FIELD_NONE;
}
const char *game_field_site_name(unsigned site) {
  static const char *names[] = {"Camp", "Moss bend", "Relay", "Stone shelf", "Old cache"};
  return site < GAME_FIELD_SITES ? names[site] : "On the trail";
}
unsigned game_field_site(const GameState *state) {
  const GameExpeditionField *field = &state->field;
  if (!field->version) return GAME_FIELD_NONE;
  for (unsigned site = 0; site < GAME_FIELD_SITES; ++site)
    if ((site != 4 || field->trace) && field->x == field->site_x[site] &&
        field->y == field->site_y[site]) return site;
  return GAME_FIELD_NONE;
}
static uint32_t random_next(uint32_t *state) {
  uint32_t value = *state;
  value ^= value << 13;
  value ^= value >> 17;
  value ^= value << 5;
  *state = value;
  return value;
}
static void corridor(uint8_t *paths, unsigned x, unsigned y, unsigned end_x,
                     unsigned end_y, int vertical_first) {
  paths[cell(x, y)] = 1;
  while (x != end_x || y != end_y) {
    if ((vertical_first && y != end_y) || x == end_x)
      y = y < end_y ? y + 1 : y - 1;
    else x = x < end_x ? x + 1 : x - 1;
    paths[cell(x, y)] = 1;
  }
}
static void generate_geometry(GameExpeditionField *field) {
  int second = (field->seed & 1u) != 0;
  const uint8_t first_x[5] = {2, 7, 2, 7, 16}, first_y[5] = {8, 8, 2, 2, 5};
  const uint8_t second_x[5] = {9, 9, 3, 15, 17}, second_y[5] = {8, 2, 5, 5, 0};
  memcpy(field->site_x, second ? second_x : first_x, 5);
  memcpy(field->site_y, second ? second_y : first_y, 5);
  const unsigned routes_a[4][2] = {{0,1},{0,2},{2,3},{3,1}};
  const unsigned routes_b[4][2] = {{0,2},{0,3},{2,1},{3,1}};
  for (unsigned route = 0; route < 4; ++route) {
    unsigned a = second ? routes_b[route][0] : routes_a[route][0];
    unsigned b = second ? routes_b[route][1] : routes_a[route][1];
    /* B's arms turn outside the central spine, preserving its distinct graph. */
    corridor(field->paths, field->site_x[a], field->site_y[a],
             field->site_x[b], field->site_y[b], second && a != 0);
  }
  corridor(field->hidden_paths, field->site_x[1], field->site_y[1],
           field->site_x[4], field->site_y[4], second);
  uint32_t terrain_random = field->seed;
  for (unsigned index = 0; index < GAME_FIELD_CELLS; ++index) {
    unsigned value = random_next(&terrain_random) % 20u;
    field->terrain[index] = value < 2 ? 2 : value < 4 ? 3 : 1;
    if (index % 20u == (second ? 11u : 10u)) field->terrain[index] = 4;
  }
}
GameResult game_field_start(GameState *state, const GameCommand *command) {
  if (state->expedition_id[0] || state->legacy_supply_encoding ||
      !state->runtime_anchors_ready || command->data.field.kind > 2 ||
      command->data.field.sample_budget > GAME_MAX_SAMPLES ||
      !command->data.field.seed) return GAME_UNAVAILABLE;
  GameExpeditionField *field = &state->field;
  memset(field, 0, sizeof(*field));
  field->version = GAME_FIELD_CONTENT_VERSION;
  field->seed = command->data.field.seed;
  field->sample_budget = (uint8_t)command->data.field.sample_budget;
  field->active_source = GAME_FIELD_NONE;
  memset(field->last_source, GAME_FIELD_NONE, sizeof(field->last_source));
  memset(field->remaining, FIELD_ATTEMPTS, sizeof(field->remaining));
  generate_geometry(field);
  field->x = field->site_x[0];
  field->y = field->site_y[0];
  field->visited = 1;
  field->walked[cell(field->x, field->y)] = 1;
  state->expedition_kind = command->data.field.kind;
  state->expedition_elapsed = 0;
  state->expedition_active = 1;
  state->expedition_last_tick = command->data.field.monotonic_seconds;
  if (state->next_identity == UINT32_MAX) return GAME_UNAVAILABLE;
  snprintf(state->expedition_id, sizeof(state->expedition_id), "BEE-E-%05u",
           state->next_identity++);
  return GAME_OK;
}
GameResult game_field_action(GameState *state, const GameCommand *command) {
  GameExpeditionField *field = &state->field;
  unsigned site = game_field_site(state);
  if (!field->version || !state->expedition_id[0]) return GAME_UNAVAILABLE;
  if (command->type == GAME_COMMAND_FIELD_MOVE) {
    int x = field->x, y = field->y;
    switch (command->data.field.direction) {
    case 0: --y;
    break;
    case 1: ++y;
    break;
    case 2: --x;
    break;
    case 3: ++x;
    break;
    default: return GAME_INVALID;
    }
    if (x < 0 || x >= 20 || y < 0 || y >= 11 ||
        !(field->paths[cell((unsigned)x,(unsigned)y)] ||
          (field->trace && field->hidden_paths[cell((unsigned)x,(unsigned)y)])))
      return GAME_UNAVAILABLE;
    field->x = (uint8_t)x;
    field->y = (uint8_t)y;
    field->walked[cell(field->x, field->y)] = 1;
    site = game_field_site(state);
    if (site < GAME_FIELD_SITES) field->visited |= (uint8_t)(1u << site);
    return GAME_OK;
  }
  if (site >= GAME_FIELD_SITES || command->data.field.site != site) return GAME_UNAVAILABLE;
  if (command->type == GAME_COMMAND_FIELD_INSPECT) {
    field->inspected |= (uint8_t)(1u << site);
    return GAME_OK;
  }
  if (!(field->inspected & (1u << site))) return GAME_UNAVAILABLE;
  if (command->type == GAME_COMMAND_FIELD_SOURCE) {
    unsigned source = command->data.field.source;
    if (source >= GAME_FIELD_SOURCES || game_field_source_site(source) != site ||
        !field->remaining[source]) return GAME_UNAVAILABLE;
    field->active_source = (uint8_t)source;
    field->last_source[game_field_source_resource(source)] = (uint8_t)source;
    state->expedition_last_tick = command->data.field.monotonic_seconds;
    return GAME_OK;
  }
  if (command->type == GAME_COMMAND_FIELD_TRACE) {
    if (site != 1 || field->trace || !field->sample_budget) return GAME_UNAVAILABLE;
    field->trace = 1;
    return GAME_OK;
  }
  if (command->type == GAME_COMMAND_FIELD_COLLECT) {
    if (site != 4 || !field->trace || field->collected || !field->sample_budget ||
        state->next_identity == UINT32_MAX) return GAME_UNAVAILABLE;
    snprintf(field->capsule_id, sizeof(field->capsule_id), "BEE-S-%05u",
             state->next_identity++);
    field->capsule_profile = (uint8_t)((GAME_MAX_SAMPLES - field->sample_budget) % 2
                            ? GAME_SAMPLE_DISCOVERY_B : GAME_SAMPLE_DISCOVERY_A);
    field->collected = 1;
    return GAME_OK;
  }
  return GAME_INVALID;
}
GameResult game_field_tick(GameState *state, uint32_t now) {
  GameExpeditionField *field = &state->field;
  if (!state->runtime_anchors_ready || now < state->expedition_last_tick)
    return GAME_INVALID;
  uint32_t elapsed = now - state->expedition_last_tick;
  state->expedition_last_tick = now;
  unsigned source = field->active_source;
  if (source >= GAME_FIELD_SOURCES || !field->remaining[source]) return elapsed ? GAME_OK : GAME_UNAVAILABLE;
  unsigned resource = game_field_source_resource(source);
  uint32_t *cargo[3] = {&state->expedition_data, &state->expedition_energy,
                       &state->expedition_essence};
  if (elapsed > 60u) elapsed = 60u;
  for (uint32_t second = 0; second < elapsed && field->remaining[source]; ++second) {
    unsigned total = *cargo[0] + *cargo[1] + *cargo[2];
    if (total >= GAME_CARGO_CAPACITY ||
        GAME_CARGO_CAPACITY - total < GAME_SUPPLY_UNIT) break;
    state->gather_progress_ms[resource] += 1000u;
    while (state->gather_progress_ms[resource] >= GAME_GATHER_ATTEMPT_MS &&
           field->remaining[source]) {
      if (state->gather_attempt_count == UINT64_MAX) return GAME_UNAVAILABLE;
      state->gather_progress_ms[resource] -= GAME_GATHER_ATTEMPT_MS;
      --field->remaining[source];
      ++field->attempts[source];
      ++state->gather_attempt_count;
      state->gather_last_attempted_mask = (uint8_t)(1u << resource);
      state->gather_last_awarded_mask = 0;
      if (random_next(&state->gather_random_state) % 4u < 3u) {
        *cargo[resource] += GAME_SUPPLY_UNIT;
        ++field->awards[source];
        state->gather_last_awarded_mask = (uint8_t)(1u << resource);
      }
      if (*cargo[0] + *cargo[1] + *cargo[2] >= GAME_CARGO_CAPACITY) break;
    }
  }
  return elapsed ? GAME_OK : GAME_UNAVAILABLE;
}
void game_field_record(const GameState *state, GameReceivedExpedition *record) {
  const GameExpeditionField *field = &state->field;
  memset(record, 0, sizeof(*record));
  record->version = field->version;
  record->seed = field->seed;
  record->kind = state->expedition_kind;
  strcpy(record->expedition_id, state->expedition_id);
  if (field->collected) strcpy(record->sample_id, field->capsule_id);
  record->cargo[0] = state->expedition_data;
  record->cargo[1] = state->expedition_energy;
  record->cargo[2] = state->expedition_essence;
  memcpy(record->terrain, field->terrain, sizeof(record->terrain));
  memcpy(record->walked, field->walked, sizeof(record->walked));
  for (unsigned site = 0; site < GAME_FIELD_SITES; ++site)
    if (field->visited & (1u << site)) {
      record->site_x[site] = field->site_x[site];
      record->site_y[site] = field->site_y[site];
    }
  record->visited = field->visited;
  record->inspected = field->inspected;
  record->trace = field->trace;
  record->collected = field->collected;
  memcpy(record->attempts, field->attempts, 6);
  memcpy(record->awards, field->awards, 6);
}
GameResult game_field_unload(GameState *state, const GameCommand *command) {
  const GameReceivedExpedition *record = command->data.field.record;
  if (!record || !state->field.version || !state->expedition_id[0] ||
      (state->field.collected && state->sample_count >= GAME_MAX_SAMPLES)) return GAME_UNAVAILABLE;
  GameReceivedExpedition expected;
  game_field_record(state, &expected);
  expected.accepted_at = record->accepted_at;
  expected.accept_sequence = command->sequence;
  if (!record->accepted_at || memcmp(&expected, record, sizeof(expected))) return GAME_CONFLICT;
  uint32_t *stock[3] = {&state->data, &state->energy, &state->essence};
  for (unsigned resource = 0; resource < 3; ++resource) {
    if (*stock[resource] > 1000000u - record->cargo[resource]) return GAME_UNAVAILABLE;
    *stock[resource] += record->cargo[resource];
  }
  if (state->field.collected) {
    GameSample *sample = &state->samples[state->sample_count];
    memset(sample, 0, sizeof(*sample));
    strcpy(sample->id, state->field.capsule_id);
    strcpy(sample->origin_expedition_id, state->expedition_id);
    sample->origin_expedition_kind = (uint8_t)state->expedition_kind;
    sample->supported_candidates = PIP_SAMPLE_CANDIDATE_MASK;
    GameSampleMetadata *metadata = &state->sample_metadata[state->sample_count];
    metadata->profile = state->field.capsule_profile;
    strcpy(metadata->content_version, PIP_DISCOVERY_CONTENT_VERSION);
    ++state->sample_count;
  }
  state->received[state->received_cursor] = *record;
  state->received_cursor = (state->received_cursor + 1u) % GAME_FIELD_HISTORY;
  if (state->received_count < GAME_FIELD_HISTORY) ++state->received_count;
  state->expedition_data = state->expedition_energy = state->expedition_essence = 0;
  state->expedition_active = 0;
  state->expedition_id[0] = 0;
  state->expedition_elapsed = 0;
  memset(&state->field, 0, sizeof(state->field));
  return GAME_OK;
}
int game_received_valid(const GameReceivedExpedition *record) {
  if (record->version != GAME_FIELD_CONTENT_VERSION || !record->seed || record->kind > 2 ||
      !record->expedition_id[0] || !memchr(record->expedition_id,0,64) ||
      !memchr(record->sample_id,0,40) || record->visited > 31 ||
      (record->inspected & ~record->visited) || record->trace > 1 || record->collected > 1 ||
      (!!record->sample_id[0] != !!record->collected) ||
      (record->collected && !record->trace)) return 0;
  uint64_t total = 0;
  for (unsigned i = 0; i < 3; ++i) {
    if (record->cargo[i] % GAME_SUPPLY_UNIT) return 0;
    total += record->cargo[i];
  }
  if (total > GAME_CARGO_CAPACITY) return 0;
  for (unsigned i = 0; i < GAME_FIELD_CELLS; ++i)
    if (record->terrain[i] > 4 || record->walked[i] > 1) return 0;
  for (unsigned i = 0; i < GAME_FIELD_SOURCES; ++i)
    if (record->attempts[i] > FIELD_ATTEMPTS || record->awards[i] > record->attempts[i]) return 0;
  for (unsigned i = 0; i < GAME_FIELD_SITES; ++i)
    if (record->site_x[i] >= 20 || record->site_y[i] >= 11 ||
        (!(record->visited & (1u << i)) && (record->site_x[i] || record->site_y[i]))) return 0;
  return 1;
}
int game_field_valid(const GameState *state) {
  const GameExpeditionField *field = &state->field;
  if (!field->version) {
    const GameExpeditionField empty = {0};
    return !memcmp(field,&empty,sizeof(empty));
  }
  if (field->version != GAME_FIELD_CONTENT_VERSION || !field->seed ||
      !state->expedition_id[0] || state->expedition_elapsed || field->x >= 20 ||
      field->y >= 11 || field->visited > 31 || (field->inspected & ~field->visited) ||
      field->trace > 1 || field->collected > 1 || field->sample_budget > 8 ||
      (field->trace && !field->sample_budget) ||
      (field->active_source >= GAME_FIELD_SOURCES && field->active_source != GAME_FIELD_NONE) ||
      !memchr(field->capsule_id,0,40) || (!!field->capsule_id[0] != !!field->collected) ||
      (field->collected && (!field->trace || field->capsule_profile < 1 || field->capsule_profile > 2))) return 0;
  /* Pinned geometry must still represent the selected connected topology.
   * This detects a checksum-valid edited corridor or hidden connector, rather
   * than merely accepting bounded bytes that could strand the player. */
  GameExpeditionField expected = {0};
  expected.seed = field->seed;
  generate_geometry(&expected);
  if (memcmp(field->terrain, expected.terrain, sizeof(field->terrain)) ||
      memcmp(field->paths, expected.paths, sizeof(field->paths)) ||
      memcmp(field->hidden_paths, expected.hidden_paths, sizeof(field->hidden_paths)) ||
      memcmp(field->site_x, expected.site_x, sizeof(field->site_x)) ||
      memcmp(field->site_y, expected.site_y, sizeof(field->site_y))) return 0;
  for (unsigned i = 0; i < GAME_FIELD_CELLS; ++i)
    if (field->terrain[i] > 4 || field->paths[i] > 1 || field->hidden_paths[i] > 1 ||
        field->walked[i] > 1 || (field->walked[i] && !field->paths[i] &&
        !(field->trace && field->hidden_paths[i]))) return 0;
  if (!field->walked[cell(field->x,field->y)]) return 0;
  for (unsigned i = 0; i < GAME_FIELD_SOURCES; ++i)
    if (field->remaining[i] + field->attempts[i] != FIELD_ATTEMPTS ||
        field->awards[i] > field->attempts[i]) return 0;
  for (unsigned i = 0; i < 3; ++i)
    if (field->last_source[i] != GAME_FIELD_NONE &&
        game_field_source_resource(field->last_source[i]) != i) return 0;
  for (unsigned i = 0; i < GAME_FIELD_SITES; ++i)
    if (field->site_x[i] >= 20 || field->site_y[i] >= 11) return 0;
  return 1;
}
