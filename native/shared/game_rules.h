#ifndef GAME_RULES_H
#define GAME_RULES_H

#include "game_state.h"

#include <stdint.h>

typedef enum {
  GAME_OK = 0,
  GAME_DUPLICATE = 1,
  GAME_INVALID = -1,
  GAME_UNAVAILABLE = -2,
  GAME_CONFLICT = -3,
  GAME_STORAGE = -4,
  GAME_COMMITTED_UNCERTAIN = 2
} GameResult;

typedef enum {
  GAME_RESOURCE_DATA = 0,
  GAME_RESOURCE_ENERGY = 1,
  GAME_RESOURCE_ESSENCE = 2
} GameResource;

typedef enum {
  GAME_COMMAND_EXPEDITION_START = 1,
  GAME_COMMAND_EXPEDITION_TICK,
  GAME_COMMAND_EXPEDITION_OFFLOAD,
  GAME_COMMAND_EXPEDITION_DISCARD,
  GAME_COMMAND_STUDY,
  GAME_COMMAND_INCUBATION_START,
  GAME_COMMAND_INCUBATION_TICK,
  GAME_COMMAND_INCUBATION_OPEN,
  GAME_COMMAND_HABITAT_VISIT,
  GAME_COMMAND_CARE_VISIT
} GameCommandType;

typedef struct {
  const char *operation_id;
  uint64_t sequence;
  GameCommandType type;
  union {
    struct {
      GameExpeditionKind kind;
      uint32_t monotonic_seconds;
    } expedition;
    uint32_t monotonic_seconds;
    struct {
      GameResource resource;
      uint32_t quantity;
      uint8_t confirm;
    } discard;
    struct {
      unsigned sample;
      unsigned study;
    } study;
    struct {
      unsigned sample;
      unsigned preference;
      uint32_t monotonic_seconds;
    } incubation;
    struct {
      unsigned sample;
      unsigned preference;
      uint32_t monotonic_seconds;
    } creation;
    struct {
      unsigned individual;
      unsigned habitat;
    } habitat;
    unsigned individual;
  } data;
} GameCommand;

/* Call once after each store load. It anchors active-only clocks and prevents
 * time while the application was stopped from advancing the simulation. */
void game_rules_resume_runtime(GameState *state, uint32_t monotonic_seconds);

/* Saves a candidate state atomically before publishing it through state.
 * sequence must be the next durable command number. Replayed older numbers
 * never apply again, even after their detailed result ages out of the journal.
 */
GameResult game_apply(const char *path, GameState *state,
                      const GameCommand *command);

#endif
