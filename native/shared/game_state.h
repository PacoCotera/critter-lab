#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stddef.h>
#include <stdint.h>

#define GAME_STATE_VERSION 1u
#define GAME_MAX_SAMPLES 8u
#define GAME_MAX_INDIVIDUALS 8u
#define GAME_OPERATION_SLOTS 32u
#define GAME_GENETIC_LOCI 5u
#define GAME_CARGO_CAPACITY 4000u
#define GAME_PACK_SIZE 1000u
#define GAME_BALANCE_VERSION "beecho-play-v1-provisional"
#define GAME_EXPEDITION_SECONDS 60u
#define GAME_INCUBATION_SECONDS 20u
#define GAME_HABITAT_COUNT 3u
#define GAME_MAX_CARE_VISITS 255u

typedef enum {
  GAME_EXPEDITION_SURVEY = 0,
  GAME_EXPEDITION_FORAGE = 1,
  GAME_EXPEDITION_RESONANCE = 2
} GameExpeditionKind;

typedef struct {
  char loci[GAME_GENETIC_LOCI][2];
  char class_id[24];
  char content_version[24];
  char rules_version[24];
} PipGenome;

typedef struct {
  char id[40];
  char origin_expedition_id[64];
  uint32_t decoded_facts;
  uint8_t origin_expedition_kind;
  uint8_t decoded_studies;
  uint8_t supported_candidates;
  uint8_t incubated;
} GameSample;

typedef struct {
  uint8_t crown;
  uint8_t eye_rings;
  uint8_t pale_markings;
  uint8_t burst_movement;
  uint8_t efficient_movement;
} PipExpression;

typedef struct {
  char id[40];
  char source_sample_id[40];
  char origin_kind[24];
  char art_id[40];
  char art_version[24];
  PipGenome genome;
  PipExpression expression;
  uint8_t habitat;
  uint8_t care_visits;
  uint8_t origin_founder;
  uint8_t art_pending;
  uint8_t revealed;
} GameIndividual;

typedef struct {
  char id[64];
  uint64_t sequence;
  uint64_t fingerprint;
} GameOperation;

typedef struct {
  uint32_t version;
  char balance_version[40];
  uint64_t revision;
  uint64_t last_operation_sequence;
  uint32_t data;
  uint32_t energy;
  uint32_t essence;
  uint32_t expedition_data;
  uint32_t expedition_energy;
  uint32_t expedition_essence;
  uint32_t expedition_elapsed;
  uint32_t expedition_last_tick;
  uint32_t expedition_kind;
  uint32_t incubation_elapsed;
  uint32_t incubation_last_tick;
  uint32_t next_identity;
  uint32_t operation_cursor;
  uint8_t expedition_active;
  uint8_t sample_count;
  uint8_t individual_count;
  uint8_t incubation_sample;
  uint8_t incubation_individual;
  uint8_t incubation_choice;
  uint8_t incubation_active;
  uint8_t incubation_ready;
  uint8_t habitat;
  uint8_t reserved;
  uint8_t runtime_anchors_ready;
  uint8_t runtime_commit_uncertain;
  char expedition_id[64];
  GameSample samples[GAME_MAX_SAMPLES];
  GameIndividual individuals[GAME_MAX_INDIVIDUALS];
  GameOperation operations[GAME_OPERATION_SLOTS];
} GameState;

void game_state_init(GameState *state);
int game_state_valid(const GameState *state);
int game_state_load(const char *path, GameState *state);
int game_state_save(const char *path, const GameState *state);

#endif
