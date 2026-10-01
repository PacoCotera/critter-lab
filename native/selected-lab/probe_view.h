#ifndef CRITTER_PROBE_VIEW_H
#define CRITTER_PROBE_VIEW_H
#include "cargo_view.h"
#include "expedition_render.h"
enum {
  PROBE_ENTRY, PROBE_MAP, PROBE_SITE, PROBE_SENT, PROBE_ENDED,
  PROBE_RETAINED, PROBE_UNAVAILABLE
};
typedef struct {
  ExpeditionFieldView field;
  CompanionCargoFacts cargo;
  unsigned phase, selector, failed, suspended, held, pressed, revision, epoch;
  unsigned action_count, focus;
  unsigned preparation_available;
  char title[64], status[96], context[96], source[96], footer[96];
  char preparation_labels[3][16];
  char actions[3][64];
} CompanionProbeView;
int kit_probe_projection(const DeviceKit *kit, CompanionProbeView *view);
unsigned probe_path_neighbors(const ExpeditionMapView *map, unsigned cell);
void probe_camera(const ExpeditionMapView *map, int width, int height, int *x, int *y);
#endif
