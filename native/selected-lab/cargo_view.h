#ifndef CRITTER_CARGO_VIEW_H
#define CRITTER_CARGO_VIEW_H
#include "kit.h"

/* Owned presentation facts, independent of a Kit or its lifetime. */
typedef struct {
  uint32_t supplies[3], delivered[3], capsules, capsule_capacity, delivered_capsules;
  unsigned accepted;
  char identity[64];
} CompanionCargoFacts;
int kit_cargo_facts(const DeviceKit *kit, CompanionCargoFacts *facts);

typedef struct {
  uint32_t supplies[3], delivered[3], capsules, capsule_capacity, delivered_capsules;
  unsigned phase, accepted, failed, focus, action_count, revision, epoch;
  int held, pressed, suspended;
  char identity[64], title[40], context[96], capsule[64], detail[192];
  char capacity[96], feedback[96], actions[2][64];
} CompanionCargoView;

int kit_cargo_projection(const DeviceKit *kit, CompanionCargoView *view);
#endif
