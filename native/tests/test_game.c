#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "game_rules.h"
#include "pip_genetics.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static int fail_second_fsync;
static unsigned transaction_fsync_count;

int fsync(int file_descriptor) {
  if (fail_second_fsync && ++transaction_fsync_count == 2u) {
    fail_second_fsync = 0;
    errno = EIO;
    return -1;
  }
  return (int)syscall(SYS_fsync, file_descriptor);
}

static GameResult apply(GameState *state, const char *path,
                        GameCommand command) {
  char operation_id[64];
  command.sequence = state->last_operation_sequence + 1u;
  (void)snprintf(operation_id, sizeof(operation_id), "test-op-%llu",
                 (unsigned long long)command.sequence);
  command.operation_id = operation_id;
  return game_apply(path, state, &command);
}

static GameCommand command(GameCommandType type) {
  GameCommand result;
  memset(&result, 0, sizeof(result));
  result.type = type;
  return result;
}

static void complete_expedition(GameState *state, const char *path,
                                GameExpeditionKind kind, uint32_t start_time) {
  unsigned before_samples = state->sample_count;
  GameCommand action = command(GAME_COMMAND_EXPEDITION_START);
  action.data.expedition.kind = kind;
  action.data.expedition.monotonic_seconds = start_time;
  assert(apply(state, path, action) == GAME_OK);
  uint32_t now = start_time;
  unsigned batches = 0;
  do {
    assert(++batches <= 2u);
    now += GAME_EXPEDITION_SECONDS;
    action = command(GAME_COMMAND_EXPEDITION_TICK);
    action.data.monotonic_seconds = now;
    assert(apply(state, path, action) == GAME_OK);
    if (state->expedition_elapsed < GAME_EXPEDITION_SECONDS) {
      action = command(GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER);
      assert(apply(state, path, action) == GAME_OK);
      action = command(GAME_COMMAND_EXPEDITION_CONTINUE);
      action.data.monotonic_seconds = now;
      assert(apply(state, path, action) == GAME_OK);
    }
  } while (state->expedition_elapsed < GAME_EXPEDITION_SECONDS);
  action = command(GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER);
  assert(apply(state, path, action) == GAME_OK);
  assert(state->sample_count == before_samples + 1u);
  assert(!state->expedition_id[0] && !state->expedition_active);
  assert(game_stock_normalized(state));
}
static void study_all(GameState *state, const char *path, unsigned sample) {
  unsigned index;
  for (index = 0; index < PIP_STUDY_COUNT; ++index) {
    GameCommand action = command(GAME_COMMAND_STUDY);
    action.data.study.sample = sample;
    action.data.study.study = index;
    assert(apply(state, path, action) == GAME_OK);
  }
  assert(state->samples[sample].decoded_studies == 0x1fu);
  assert(state->samples[sample].decoded_facts == PIP_REQUIRED_FACTS_MASK);
}

static void start_and_open(GameState *state, const char *path, unsigned sample,
                           unsigned preference, uint32_t start_time) {
  GameCommand action = command(GAME_COMMAND_INCUBATION_START);
  unsigned individual = state->individual_count;
  action.data.creation.sample = sample;
  action.data.creation.preference = preference;
  action.data.creation.monotonic_seconds = start_time;
  assert(apply(state, path, action) == GAME_OK);
  assert(state->individual_count == individual + 1u);
  assert(!state->individuals[individual].revealed);
  assert(state->individuals[individual].origin_founder);
  assert(strcmp(state->individuals[individual].source_sample_id,
                state->samples[sample].id) == 0);
  action = command(GAME_COMMAND_INCUBATION_OPEN);
  assert(apply(state, path, action) == GAME_UNAVAILABLE);
  action = command(GAME_COMMAND_INCUBATION_TICK);
  action.data.monotonic_seconds = start_time + GAME_INCUBATION_SECONDS - 1u;
  assert(apply(state, path, action) == GAME_OK);
  assert(!state->incubation_ready);
  action = command(GAME_COMMAND_INCUBATION_OPEN);
  assert(apply(state, path, action) == GAME_UNAVAILABLE);
  action = command(GAME_COMMAND_INCUBATION_TICK);
  action.data.monotonic_seconds = start_time + GAME_INCUBATION_SECONDS;
  assert(apply(state, path, action) == GAME_OK);
  assert(state->incubation_ready);
  action = command(GAME_COMMAND_INCUBATION_OPEN);
  assert(apply(state, path, action) == GAME_OK);
  assert(state->individuals[individual].revealed);
}

/* Write the historical same-ABI payload without the appended V2 fields. This
 * deliberately does not call the current writer, whose header must be V2. */
static void write_legacy_save(const char *path, const GameState *state) {
  struct {
    char magic[8];
    uint32_t version;
    uint32_t payload_size;
    uint32_t checksum;
    GameState state;
  } saved;
  memset(&saved, 0, sizeof(saved));
  memcpy(saved.magic, "BEECHOV1", 8u);
  saved.version = 1u;
  saved.payload_size = (uint32_t)offsetof(GameState, gather_progress_ms);
  saved.state = *state;
  saved.state.version = 1u;
  saved.state.runtime_anchors_ready = 0;
  saved.state.expedition_last_tick = 0;
  saved.state.incubation_last_tick = 0;
  const unsigned char *bytes = (const unsigned char *)&saved.state;
  saved.checksum = 2166136261u;
  for (size_t i = 0; i < saved.payload_size; ++i) {
    saved.checksum ^= bytes[i];
    saved.checksum *= 16777619u;
  }
  FILE *file = fopen(path, "wb");
  assert(file);
  size_t length = offsetof(GameState, gather_progress_ms) +
                  (size_t)((unsigned char *)&saved.state - (unsigned char *)&saved);
  assert(fwrite(&saved, 1, length, file) == length);
  assert(fclose(file) == 0);
}

static void start_gathering(GameState *state, const char *path, uint32_t now) {
  game_rules_resume_runtime(state, now);
  GameCommand action = command(GAME_COMMAND_EXPEDITION_START);
  action.data.expedition.kind = GAME_EXPEDITION_FORAGE;
  action.data.expedition.monotonic_seconds = now;
  assert(apply(state, path, action) == GAME_OK);
}

static void tick_gathering(GameState *state, const char *path, uint32_t now) {
  GameCommand action = command(GAME_COMMAND_EXPEDITION_TICK);
  action.data.monotonic_seconds = now;
  assert(apply(state, path, action) == GAME_OK);
}

static void test_whole_supply_rules(const char *path) {
  GameState state;
  GameState reopened;
  GameState unchanged;
  GameCommand action;
  assert(GAME_COMMAND_EXPEDITION_OFFLOAD == 3);
  assert(GAME_COMMAND_EXPEDITION_TRANSFER == 11);
  assert(GAME_COMMAND_STOCK_NORMALIZE == 12);
  assert(GAME_COMMAND_EXPEDITION_CONTINUE == 13);
  assert(GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER == 14);
  assert(GAME_COMMAND_EXPEDITION_FINISH == 15);

  /* A legacy file is decoded without modifying raw inventory or its source
   * file. Its pending encoding flag survives a V2 save and restart. */
  game_state_init(&state);
  strcpy(state.expedition_id, "BEE-E-90001");
  state.expedition_elapsed = 5u;
  state.expedition_data = state.expedition_energy = state.expedition_essence = 110u;
  write_legacy_save(path, &state);
  assert(game_state_load(path, &state) == 0);
  int legacy_file = open(path, O_RDONLY);
  uint32_t source_version = 0;
  assert(legacy_file >= 0);
  assert(pread(legacy_file, &source_version, sizeof(source_version), 8) ==
         (ssize_t)sizeof(source_version));
  close(legacy_file);
  assert(source_version == 1u); /* Decoding did not rewrite the old file. */
  assert(state.version == GAME_STATE_VERSION);
  assert(game_supply_conversion_pending(&state));
  assert(state.expedition_data == 110u && !game_stock_normalized(&state));
  assert(game_state_save(path, &state) == 0);
  assert(game_state_load(path, &reopened) == 0);
  state = reopened;
  assert(game_supply_conversion_pending(&state));
  action = command(GAME_COMMAND_EXPEDITION_OFFLOAD);
  action.sequence = 1u;
  action.operation_id = "legacy-offload-reserved";
  assert(game_apply(path, &state, &action) == GAME_OK);
  assert(state.operations[0].fingerprint == UINT64_C(13000014988436593482));
  assert(state.data == 110u && !state.expedition_id[0]);
  assert(game_state_load(path, &reopened) == 0);
  state = reopened;
  assert(game_apply(path, &state, &action) == GAME_DUPLICATE);
  GameCommand wrong_policy = action;
  wrong_policy.type = GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER;
  assert(game_apply(path, &state, &wrong_policy) == GAME_CONFLICT);
  action = command(GAME_COMMAND_STOCK_NORMALIZE);
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.data == 100u && state.energy == 100u && state.essence == 100u);
  assert(state.expedition_data == 0u && state.gather_progress_ms[0] == 400u);
  assert(!state.expedition_id[0] && state.expedition_elapsed == 0u);
  assert(game_stock_normalized(&state));
  unchanged = state;
  assert(apply(&state, path, action) == GAME_DUPLICATE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);

  /* A reserved V2 command11 keeps its raw split behavior. Conversion happens
   * only after that receipt has been settled, without inventing an item. */
  game_state_init(&state);
  state.legacy_supply_encoding = 1;
  state.data = 99u;
  state.expedition_data = 110u;
  state.expedition_elapsed = 5u;
  strcpy(state.expedition_id, "BEE-E-90002");
  action = command(GAME_COMMAND_EXPEDITION_TRANSFER);
  action.sequence = 1u;
  action.operation_id = "legacy-split-reserved";
  assert(game_apply(path, &state, &action) == GAME_OK);
  assert(state.operations[0].fingerprint == UINT64_C(6414028411195858737));
  assert(state.data == 100u && state.expedition_data == 109u);
  assert(state.expedition_elapsed == 5u && state.expedition_id[0]);
  assert(game_state_load(path, &reopened) == 0);
  state = reopened;
  assert(game_apply(path, &state, &action) == GAME_DUPLICATE);
  action = command(GAME_COMMAND_STOCK_NORMALIZE);
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.expedition_data == 100u && state.gather_progress_ms[0] == 360u);

  /* Combining two old residues yields historical time credit, never a whole
   * inventory item. The credit uses no cargo capacity, even above one interval. */
  game_state_init(&state);
  state.legacy_supply_encoding = 1;
  state.data = 99u;
  state.expedition_data = 99u;
  action = command(GAME_COMMAND_STOCK_NORMALIZE);
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.data == 0u && state.expedition_data == 0u);
  assert(state.gather_progress_ms[0] == 7920u);
  assert(game_gather_remaining_ms(&state) == 0u);
  assert(game_gather_due_mask(&state) == 1u);
  assert(game_gather_required_slots(&state) == 1u);
  assert(!game_gather_capacity_blocked(&state));
  state.gather_random_state = 1u;
  start_gathering(&state, path, 100u);
  tick_gathering(&state, path, 101u);
  assert(state.expedition_data == 100u && state.expedition_energy == 0u);
  assert(state.gather_progress_ms[0] == 4920u && state.gather_attempt_count == 1u);
  tick_gathering(&state, path, 102u);
  assert(state.expedition_data == 200u && state.gather_progress_ms[0] == 1920u);
  game_state_init(&state);
  state.legacy_supply_encoding = 1;
  state.data = state.energy = state.essence = 99u;
  state.expedition_data = 1334u;
  state.expedition_energy = state.expedition_essence = 1333u;
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.expedition_data + state.expedition_energy +
         state.expedition_essence == 3900u);
  assert(state.gather_progress_ms[0] == 5320u);
  assert(state.gather_progress_ms[1] == 5280u);
  assert(game_gather_capacity_blocked(&state));

  /* Fresh acceptance of a sealed old partial-only haul can convert atomically.
   * A normal new empty send remains unavailable; its preparation stays here. */
  game_state_init(&state);
  state.legacy_supply_encoding = 1;
  state.expedition_data = 44u;
  state.expedition_elapsed = 2u;
  strcpy(state.expedition_id, "BEE-E-90003");
  assert(!game_transfer_available(&state));
  action = command(GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER);
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.data == 0u && state.expedition_data == 0u);
  assert(state.gather_progress_ms[0] == 1760u);
  assert(!state.expedition_active && state.expedition_elapsed == 2u);
  assert(strcmp(state.expedition_id, "BEE-E-90003") == 0);
  assert(state.sample_count == 0u);
  unchanged = state;
  assert(apply(&state, path, action) == GAME_UNAVAILABLE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);
  assert(game_gather_due_mask(&state) == 1u);
  assert(game_gather_remaining_ms(&state) == 2240u);
  assert(game_state_load(path, &reopened) == 0);
  game_rules_resume_runtime(&reopened, 900u);
  state = reopened;
  assert(state.gather_progress_ms[0] == 1760u &&
         state.gather_random_state == GAME_GATHER_INITIAL_RANDOM_STATE);
  action = command(GAME_COMMAND_EXPEDITION_CONTINUE);
  action.data.monotonic_seconds = 900u;
  assert(apply(&state, path, action) == GAME_OK);
  tick_gathering(&state, path, 903u);
  assert(state.gather_attempt_count == 1u &&
         state.gather_last_attempted_mask == 1u);
  assert(state.gather_last_awarded_mask == 0u && state.expedition_data == 0u &&
         state.gather_random_state == 1085196063u);
  assert(state.gather_progress_ms[0] == 760u &&
         state.gather_progress_ms[1] == 3000u);
  assert(game_gather_due_mask(&state) == 6u &&
         game_gather_remaining_ms(&state) == 1000u);

  /* Seed1 makes the first three class attempts succeed; the next Data attempt
   * misses. Three seconds are only preparation, not partly collected items. */
  game_state_init(&state);
  state.gather_random_state = 1u;
  start_gathering(&state, path, 100u);
  tick_gathering(&state, path, 103u);
  assert(state.expedition_data == 0u && !game_transfer_available(&state));
  assert(state.gather_progress_ms[0] == 3000u);
  assert(game_gather_remaining_ms(&state) == 1000u);
  assert(game_gather_due_mask(&state) == 7u);
  assert(state.gather_attempt_count == 0u && state.gather_random_state == 1u);
  action = command(GAME_COMMAND_EXPEDITION_TICK);
  action.data.monotonic_seconds = 104u;
  action.sequence = state.last_operation_sequence + 1u;
  action.operation_id = "chance-save-failure";
  char missing_path[256];
  assert(snprintf(missing_path, sizeof(missing_path), "%s/missing/save", path) > 0);
  unchanged = state;
  assert(game_apply(missing_path, &state, &action) == GAME_STORAGE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);
  tick_gathering(&state, path, 104u);
  assert(state.expedition_data == 100u && state.expedition_energy == 100u &&
         state.expedition_essence == 100u);
  assert(state.gather_attempt_count == 3u);
  assert(state.gather_last_attempted_mask == 7u &&
         state.gather_last_awarded_mask == 7u);
  tick_gathering(&state, path, 108u);
  assert(state.expedition_data == 100u && state.expedition_energy == 200u &&
         state.expedition_essence == 200u);
  assert(state.gather_attempt_count == 6u && state.gather_last_awarded_mask == 6u);
  tick_gathering(&state, path, 109u);
  assert(state.gather_progress_ms[0] == 1000u);
  assert(state.gather_last_awarded_mask == 6u);
  uint32_t random_before_transfer = state.gather_random_state;
  uint64_t attempts_before_transfer = state.gather_attempt_count;
  char expedition_id[64];
  strcpy(expedition_id, state.expedition_id);
  action = command(GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER);
  action.sequence = state.last_operation_sequence + 1u;
  action.operation_id = "whole-early-haul";
  assert(game_apply(path, &state, &action) == GAME_OK);
  assert(state.data == 100u && state.energy == 200u && state.essence == 200u);
  assert(state.expedition_data == 0u && state.gather_progress_ms[0] == 1000u);
  assert(state.gather_random_state == random_before_transfer &&
         state.gather_attempt_count == attempts_before_transfer);
  assert(!state.expedition_active && state.expedition_elapsed == 9u &&
         strcmp(state.expedition_id, expedition_id) == 0);
  assert(game_state_load(path, &reopened) == 0);
  game_rules_resume_runtime(&reopened, 900u);
  state = reopened;
  unchanged = state;
  assert(game_apply(path, &state, &action) == GAME_DUPLICATE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);
  action = command(GAME_COMMAND_EXPEDITION_CONTINUE);
  action.data.monotonic_seconds = 900u;
  assert(apply(&state, path, action) == GAME_OK);
  tick_gathering(&state, path, 903u);
  assert(state.expedition_elapsed == 12u && state.gather_attempt_count == 9u);
  assert(strcmp(state.expedition_id, expedition_id) == 0);

  /* Restart and different tick batching produce the same opportunity sequence.
   * Downtime is anchored away; replay cannot redraw a committed chance result. */
  game_state_init(&state);
  state.gather_random_state = 1u;
  start_gathering(&state, path, 100u);
  tick_gathering(&state, path, 112u);
  GameState uninterrupted = state;
  game_state_init(&state);
  state.gather_random_state = 1u;
  start_gathering(&state, path, 100u);
  tick_gathering(&state, path, 103u);
  assert(game_state_load(path, &reopened) == 0);
  game_rules_resume_runtime(&reopened, 900u);
  state = reopened;
  tick_gathering(&state, path, 909u);
  assert(state.expedition_data == uninterrupted.expedition_data);
  assert(state.expedition_energy == uninterrupted.expedition_energy);
  assert(state.expedition_essence == uninterrupted.expedition_essence);
  assert(state.gather_random_state == uninterrupted.gather_random_state);
  assert(state.gather_attempt_count == uninterrupted.gather_attempt_count);
  assert(memcmp(state.gather_progress_ms, uninterrupted.gather_progress_ms,
                sizeof(state.gather_progress_ms)) == 0);
  action = command(GAME_COMMAND_EXPEDITION_TICK);
  action.data.monotonic_seconds = 909u;
  action.sequence = state.last_operation_sequence + 1u;
  action.operation_id = "chance-tick-retry";
  assert(game_apply(path, &state, &action) == GAME_OK);
  unchanged = state;
  assert(game_apply(path, &state, &action) == GAME_DUPLICATE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);

  /* Near-full capacity reserves all possible awards BEFORE time and RNG. After
   * acceptance+continuation, the same next outcome awards all three classes. */
  game_state_init(&state);
  state.gather_random_state = 1u;
  state.expedition_data = 3800u;
  start_gathering(&state, path, 100u);
  tick_gathering(&state, path, 104u);
  assert(state.expedition_elapsed == 3u && state.gather_progress_ms[0] == 3000u);
  assert(state.gather_attempt_count == 0u && state.gather_random_state == 1u);
  assert(game_gather_capacity_blocked(&state));
  unchanged = state;
  action = command(GAME_COMMAND_EXPEDITION_TICK);
  action.data.monotonic_seconds = 105u;
  assert(apply(&state, path, action) == GAME_UNAVAILABLE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);
  action = command(GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER);
  assert(apply(&state, path, action) == GAME_OK);
  action = command(GAME_COMMAND_EXPEDITION_CONTINUE);
  action.data.monotonic_seconds = 900u;
  assert(apply(&state, path, action) == GAME_OK);
  tick_gathering(&state, path, 901u);
  assert(state.expedition_data == 100u && state.expedition_energy == 100u &&
         state.expedition_essence == 100u && state.gather_attempt_count == 3u);
  assert(state.expedition_elapsed == 4u && state.data == 3800u);
  game_state_init(&state);
  state.expedition_data = GAME_CARGO_CAPACITY;
  start_gathering(&state, path, 100u);
  unchanged = state;
  action = command(GAME_COMMAND_EXPEDITION_TICK);
  action.data.monotonic_seconds = 101u;
  assert(apply(&state, path, action) == GAME_UNAVAILABLE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);

  /* Completion awards a sample independently of loot. With a full shelf and
   * no items, FINISH releases the route; a new route keeps preparation. */
  game_state_init(&state);
  strcpy(state.expedition_id, "BEE-E-SAMPLE-ONLY");
  state.expedition_elapsed = GAME_EXPEDITION_SECONDS;
  state.gather_progress_ms[0] = 2000u;
  assert(game_transfer_available(&state));
  action = command(GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER);
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.sample_count == 1u && !state.expedition_id[0]);
  assert(state.gather_progress_ms[0] == 2000u);
  for (unsigned i = 1; i < GAME_MAX_SAMPLES; ++i) {
    state.samples[i] = state.samples[0];
    assert(snprintf(state.samples[i].id, sizeof(state.samples[i].id),
                    "BEE-S-SHELF-%u", i) > 0);
  }
  state.sample_count = GAME_MAX_SAMPLES;
  strcpy(state.expedition_id, "BEE-E-FULL-SHELF");
  state.expedition_elapsed = GAME_EXPEDITION_SECONDS;
  assert(game_state_valid(&state) && !game_transfer_available(&state));
  unchanged = state;
  assert(apply(&state, path, action) == GAME_UNAVAILABLE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);
  action = command(GAME_COMMAND_EXPEDITION_FINISH);
  assert(apply(&state, path, action) == GAME_OK);
  assert(!state.expedition_id[0] && state.gather_progress_ms[0] == 2000u);
  start_gathering(&state, path, 1000u);
  assert(state.gather_progress_ms[0] == 2000u);
  assert(strcmp(state.expedition_id, "BEE-E-FULL-SHELF") != 0);

  /* Paid Lab actions are gated by encoding, even when old stocks are multiples
   * of100. Converted validation rejects fractional inventory and discard. */
  game_state_init(&state);
  strcpy(state.expedition_id, "BEE-E-SPENDING");
  state.expedition_elapsed = GAME_EXPEDITION_SECONDS;
  action = command(GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER);
  assert(apply(&state, path, action) == GAME_OK);
  state.data = state.energy = state.essence = 3000u;
  state.legacy_supply_encoding = 1;
  action = command(GAME_COMMAND_STUDY);
  action.data.study.sample = 0u;
  action.data.study.study = 0u;
  unchanged = state;
  assert(apply(&state, path, action) == GAME_UNAVAILABLE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);
  action = command(GAME_COMMAND_STOCK_NORMALIZE);
  assert(apply(&state, path, action) == GAME_OK);
  study_all(&state, path, 0u);
  state.legacy_supply_encoding = 1;
  action = command(GAME_COMMAND_INCUBATION_START);
  action.data.creation.sample = 0u;
  unchanged = state;
  assert(apply(&state, path, action) == GAME_UNAVAILABLE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);
  state.legacy_supply_encoding = 0;
  ++state.expedition_data;
  assert(!game_state_valid(&state));
  --state.expedition_data;
  start_gathering(&state, path, 100u);
  state.expedition_data = 100u;
  action = command(GAME_COMMAND_EXPEDITION_DISCARD);
  action.data.discard.resource = GAME_RESOURCE_DATA;
  action.data.discard.quantity = 1u;
  action.data.discard.confirm = 1u;
  unchanged = state;
  assert(apply(&state, path, action) == GAME_INVALID);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);

  /* Idle items can transfer without a fake sample. A stock overflow rejects
   * the whole candidate, including conversion and its preparation credits. */
  game_state_init(&state);
  state.expedition_data = 100u;
  action = command(GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER);
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.data == 100u && state.sample_count == 0u);
  state.legacy_supply_encoding = 1;
  state.data = 1000000u;
  state.expedition_data = 199u;
  unchanged = state;
  assert(apply(&state, path, action) == GAME_UNAVAILABLE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);
}

int main(void) {
  char path[] = "/tmp/beecho-game-XXXXXX";
  GameState state;
  GameState reopened;
  GameCommand action;
  int file = mkstemp(path);
  unsigned first_individual;
  unsigned index;
  assert(file >= 0);
  close(file);
  unlink(path);
  assert(game_state_load(path, &state) == 1);
  game_state_init(&state);
  assert(game_state_save(path, &state) == 0);
  assert(game_state_load(path, &reopened) == 0);
  state = reopened;
  game_rules_resume_runtime(&state, 100u);

  /* An early return transfers its gathered resources but grants no sample. */
  action = command(GAME_COMMAND_EXPEDITION_START);
  action.data.expedition.kind = GAME_EXPEDITION_SURVEY;
  action.data.expedition.monotonic_seconds = 100u;
  action.sequence = state.last_operation_sequence + 1u;
  action.operation_id = "pre-rename-failure";
  GameState unchanged = state;
  char unavailable_path[256];
  assert(snprintf(unavailable_path, sizeof(unavailable_path), "%s/missing/save",
                  path) > 0);
  assert(game_apply(unavailable_path, &state, &action) == GAME_STORAGE);
  assert(memcmp(&state, &unchanged, sizeof(state)) == 0);
  assert(game_apply(path, &state, &action) == GAME_OK);
  action = command(GAME_COMMAND_EXPEDITION_TICK);
  action.data.monotonic_seconds = 110u;
  assert(apply(&state, path, action) == GAME_OK);
  action = command(GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER);
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.sample_count == 0 && state.expedition_elapsed == 10u);
  assert(game_stock_normalized(&state) &&
         state.data + state.energy + state.essence > 0u);

  /* Continue the same source expedition after its early receipt. Samples are
   * awarded at completed acceptance, independently of the gathering outcomes. */
  action = command(GAME_COMMAND_EXPEDITION_CONTINUE);
  action.data.monotonic_seconds = 110u;
  assert(apply(&state, path, action) == GAME_OK);
  action = command(GAME_COMMAND_EXPEDITION_TICK);
  action.data.monotonic_seconds = 160u;
  assert(apply(&state, path, action) == GAME_OK);
  action = command(GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER);
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.sample_count == 1);
  assert(state.samples[0].origin_expedition_kind == GAME_EXPEDITION_SURVEY);
  assert(game_stock_normalized(&state));
  complete_expedition(&state, path, GAME_EXPEDITION_FORAGE, 170u);
  /* Existing knowledge does not choose the genotype. Choice is explicit. */
  study_all(&state, path, 0u);
  assert(state.samples[0].incubated == 0);
  complete_expedition(&state, path, GAME_EXPEDITION_RESONANCE, 230u);
  assert(state.sample_count == 3);
  start_and_open(&state, path, 0u, 0u, 290u);
  first_individual = state.individual_count - 1u;
  assert(state.individuals[first_individual].genome.loci[2][0] == 'P');
  assert(strcmp(state.individuals[first_individual].art_id,
                "design/v1-pip/pip-carried.png") == 0);
  assert(!state.individuals[first_individual].expression.pale_markings);
  assert(state.individuals[first_individual].expression.efficient_movement);

  action = command(GAME_COMMAND_HABITAT_VISIT);
  action.data.habitat.individual = first_individual;
  action.data.habitat.habitat = 1u;
  assert(apply(&state, path, action) == GAME_OK);
  action = command(GAME_COMMAND_CARE_VISIT);
  action.data.individual = first_individual;
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.individuals[first_individual].care_visits == 1u);

  /* Restart during incubation resets the timer anchor; downtime adds nothing.
   */
  complete_expedition(&state, path, GAME_EXPEDITION_SURVEY, 310u);
  assert(state.sample_count == 4);
  study_all(&state, path, 1u);
  action = command(GAME_COMMAND_INCUBATION_START);
  action.data.creation.sample = 1u;
  action.data.creation.preference = 1u;
  action.data.creation.monotonic_seconds = 370u;
  assert(apply(&state, path, action) == GAME_OK);
  assert(!state.individuals[1].revealed);
  assert(state.individuals[1].genome.loci[2][0] == 'p');
  assert(state.individuals[1].expression.pale_markings);
  assert(game_state_save(path, &state) == 0);
  assert(game_state_load(path, &reopened) == 0);
  assert(reopened.incubation_elapsed == 0);
  game_rules_resume_runtime(&reopened, 900u);
  state = reopened;
  action = command(GAME_COMMAND_INCUBATION_TICK);
  action.data.monotonic_seconds = 919u;
  assert(apply(&state, path, action) == GAME_OK);
  assert(!state.incubation_ready);
  action = command(GAME_COMMAND_INCUBATION_OPEN);
  assert(apply(&state, path, action) == GAME_UNAVAILABLE);
  action = command(GAME_COMMAND_INCUBATION_TICK);
  action.data.monotonic_seconds = 920u;
  assert(apply(&state, path, action) == GAME_OK);
  action = command(GAME_COMMAND_INCUBATION_OPEN);
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.individual_count == 2 && state.individuals[1].revealed);

  /* If rename succeeds but directory fsync fails, the operation is committed
   * and visible in memory, but the process must stop writes until reloaded. */
  action = command(GAME_COMMAND_CARE_VISIT);
  action.data.individual = 1u;
  action.sequence = state.last_operation_sequence + 1u;
  action.operation_id = "directory-sync-uncertain";
  transaction_fsync_count = 0;
  fail_second_fsync = 1;
  assert(game_apply(path, &state, &action) == GAME_COMMITTED_UNCERTAIN);
  assert(!fail_second_fsync);
  assert(state.runtime_commit_uncertain);
  assert(state.individuals[1].care_visits == 1u);
  assert(game_apply(path, &state, &action) == GAME_DUPLICATE);
  GameCommand blocked = command(GAME_COMMAND_CARE_VISIT);
  blocked.data.individual = 1u;
  blocked.sequence = state.last_operation_sequence + 1u;
  blocked.operation_id = "blocked-until-reload";
  assert(game_apply(path, &state, &blocked) == GAME_COMMITTED_UNCERTAIN);
  assert(game_state_load(path, &reopened) == 0);
  assert(reopened.last_operation_sequence == state.last_operation_sequence);
  assert(reopened.individuals[1].care_visits == 1u);
  state = reopened;

  /* A durable operation retry is a no-op; stale retries stay inert after
   * their detail slot rotates out of the bounded journal. */
  action = command(GAME_COMMAND_CARE_VISIT);
  action.data.individual = 1u;
  action.sequence = state.last_operation_sequence + 1u;
  action.operation_id = "care-retry-check";
  assert(game_apply(path, &state, &action) == GAME_OK);
  assert(game_apply(path, &state, &action) == GAME_DUPLICATE);
  GameCommand stale_retry = action;
  for (index = 0; index < GAME_OPERATION_SLOTS + 1u; ++index) {
    action = command(GAME_COMMAND_CARE_VISIT);
    action.data.individual = 1u;
    assert(apply(&state, path, action) == GAME_OK);
  }
  assert(game_apply(path, &state, &stale_retry) == GAME_DUPLICATE);
  assert(game_state_load(path, &reopened) == 0);
  assert(reopened.last_operation_sequence == state.last_operation_sequence);

  /* Corrupt bytes and unsupported versions fail closed. */
  file = open(path, O_RDWR);
  assert(file >= 0);
  unsigned char changed = 0xffu;
  assert(pwrite(file, &changed, 1u, 16) == 1);
  close(file);
  assert(game_state_load(path, &reopened) == -1);
  test_whole_supply_rules(path);
  unlink(path);
  char lock_path[256];
  char temporary_path[256];
  assert(snprintf(lock_path, sizeof(lock_path), "%s.lock", path) > 0);
  assert(snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path) > 0);
  unlink(lock_path);
  unlink(temporary_path);
  return 0;
}
