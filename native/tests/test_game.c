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
  uint32_t before_data = state->data;
  uint32_t before_energy = state->energy;
  uint32_t before_essence = state->essence;
  unsigned before_samples = state->sample_count;
  GameCommand action = command(GAME_COMMAND_EXPEDITION_START);
  action.data.expedition.kind = kind;
  action.data.expedition.monotonic_seconds = start_time;
  assert(apply(state, path, action) == GAME_OK);
  action = command(GAME_COMMAND_EXPEDITION_TICK);
  action.data.monotonic_seconds = start_time + GAME_EXPEDITION_SECONDS;
  assert(apply(state, path, action) == GAME_OK);
  assert(state->expedition_elapsed == GAME_EXPEDITION_SECONDS);
  action = command(GAME_COMMAND_EXPEDITION_OFFLOAD);
  assert(apply(state, path, action) == GAME_OK);
  assert(state->sample_count == before_samples + 1u);
  assert(state->data == before_data + GAME_EXPEDITION_SECONDS * 22u);
  assert(state->energy == before_energy + GAME_EXPEDITION_SECONDS * 22u);
  assert(state->essence == before_essence + GAME_EXPEDITION_SECONDS * 22u);
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
  action = command(GAME_COMMAND_EXPEDITION_OFFLOAD);
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.sample_count == 0);
  assert(state.data == 220u && state.energy == 220u && state.essence == 220u);

  /* A complete active expedition gives stock and one stable sample. */
  action = command(GAME_COMMAND_EXPEDITION_START);
  action.data.expedition.kind = GAME_EXPEDITION_FORAGE;
  action.data.expedition.monotonic_seconds = 110u;
  assert(apply(&state, path, action) == GAME_OK);
  action = command(GAME_COMMAND_EXPEDITION_TICK);
  action.data.monotonic_seconds = 170u;
  assert(apply(&state, path, action) == GAME_OK);
  action = command(GAME_COMMAND_EXPEDITION_OFFLOAD);
  assert(apply(&state, path, action) == GAME_OK);
  assert(state.sample_count == 1);
  assert(state.samples[0].origin_expedition_kind == GAME_EXPEDITION_FORAGE);
  assert(state.data == 1540u && state.energy == 1540u &&
         state.essence == 1540u);

  /* Existing knowledge does not choose the genotype. Choice is explicit. */
  study_all(&state, path, 0u);
  assert(state.samples[0].incubated == 0);
  complete_expedition(&state, path, GAME_EXPEDITION_RESONANCE, 170u);
  assert(state.sample_count == 2);
  start_and_open(&state, path, 0u, 0u, 230u);
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
  complete_expedition(&state, path, GAME_EXPEDITION_SURVEY, 250u);
  assert(state.sample_count == 3);
  study_all(&state, path, 1u);
  action = command(GAME_COMMAND_INCUBATION_START);
  action.data.creation.sample = 1u;
  action.data.creation.preference = 1u;
  action.data.creation.monotonic_seconds = 310u;
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
  unlink(path);
  char lock_path[256];
  char temporary_path[256];
  assert(snprintf(lock_path, sizeof(lock_path), "%s.lock", path) > 0);
  assert(snprintf(temporary_path, sizeof(temporary_path), "%s.tmp", path) > 0);
  unlink(lock_path);
  unlink(temporary_path);
  return 0;
}
