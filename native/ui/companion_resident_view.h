#ifndef CRITTER_COMPANION_RESIDENT_VIEW_H
#define CRITTER_COMPANION_RESIDENT_VIEW_H
#include <stdint.h>

enum { RESIDENT_PORTRAIT_PENDING, RESIDENT_PORTRAIT_PLAIN,
       RESIDENT_PORTRAIT_MARKED, RESIDENT_EMPTY_HABITAT };

/* Copied preview facts only. Selection, cache authority and visits stay outside
 * presentation; an image slot is not a genomic or appearance decision. */
typedef struct {
  uint32_t count, selected_index, visits;
  unsigned portrait, failed, current, online;
  char identity[40], coat[40], property[96], status[96], feedback[128];
} CompanionResidentView;
#endif
