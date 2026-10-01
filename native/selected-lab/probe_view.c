#include "probe_view.h"
#include <stdio.h>
#include <string.h>

unsigned probe_path_neighbors(const ExpeditionMapView *map, unsigned cell) {
  if (!map || cell >= EXPEDITION_MAP_CELLS || !map->paths[cell]) return 0;
  unsigned x = cell % EXPEDITION_MAP_COLUMNS, y = cell / EXPEDITION_MAP_COLUMNS;
  return (y && map->paths[cell - EXPEDITION_MAP_COLUMNS] ? 1u : 0u) |
         (x + 1 < EXPEDITION_MAP_COLUMNS && map->paths[cell + 1] ? 2u : 0u) |
         (y + 1 < EXPEDITION_MAP_ROWS && map->paths[cell + EXPEDITION_MAP_COLUMNS] ? 4u : 0u) |
         (x && map->paths[cell - 1] ? 8u : 0u);
}
void probe_camera(const ExpeditionMapView *map, int width, int height, int *x, int *y) {
  int center_x = map->avatar_x * 32 + 16 - width / 2;
  int center_y = map->avatar_y * 32 + 16 - height / 2;
  int max_x = EXPEDITION_MAP_COLUMNS * 32 - width;
  int max_y = EXPEDITION_MAP_ROWS * 32 - height;
  *x = center_x < 0 ? 0 : center_x > max_x ? max_x : center_x;
  *y = center_y < 0 ? 0 : center_y > max_y ? max_y : center_y;
}
int kit_probe_projection(const DeviceKit *kit, CompanionProbeView *out) {
  if (!kit || !out || !kit->lab) return 0;
  const KitView *view = &kit->companion;
  int selector = view->page == COMP_MODES && view->mode == COMP_PROBE;
  if (!selector && view->page != COMP_PROBE && view->page != COMP_FIELD_SITE) return 0;
  memset(out, 0, sizeof(*out));
  out->selector = selector;
  out->failed = kit->failed || kit->lab->storage_error;
  out->suspended = view->suspended;
  out->revision = view->revision;
  out->epoch = view->epoch;
  out->focus = view->focus;
  for (unsigned button = 0; button < 10; ++button) {
    out->held |= view->gestures[button].held;
    out->pressed |= view->gestures[button].held && view->gestures[button].allowed;
  }
  if (!kit_cargo_facts(kit, &out->cargo)) return 0;
  int has_field = kit_field_projection(kit, &out->field);
  const GameState *game = &kit->lab->game;
  unsigned transfer = kit->journal.phase;
  int sealed = transfer >= KIT_WAITING && transfer <= KIT_ACK_PENDING;
  int live = has_field && game->field.version && game->expedition_id[0] &&
             out->field.map.avatar_visible && !sealed && !out->cargo.accepted;
  /* Preparation belongs to the Companion across outings and transfers. Never
   * reconstruct it from a sealed delivery record or reset it with the map. */
  int saved_preparation = 0;
  for (unsigned resource = 0; resource < 3; ++resource) {
    out->field.preparation_ms[resource] = game->gather_progress_ms[resource];
    saved_preparation |= game->gather_progress_ms[resource] != 0;
    if (!live || (out->field.preparation_status[resource] == EXPEDITION_PREP_NOT_STARTED &&
                  game->gather_progress_ms[resource]))
      out->field.preparation_status[resource] = game->gather_progress_ms[resource]
          ? EXPEDITION_PREP_PAUSED : EXPEDITION_PREP_NOT_STARTED;
  }
  out->preparation_available = !out->failed || live || saved_preparation;
  out->phase = out->failed ? PROBE_UNAVAILABLE : out->cargo.accepted ? PROBE_ENDED :
      sealed ? PROBE_SENT : live ? (view->page == COMP_FIELD_SITE ? PROBE_SITE : PROBE_MAP) :
      game->expedition_id[0] ? PROBE_RETAINED : PROBE_ENTRY;
  const char *title = live ? out->field.location : out->phase == PROBE_ENDED ? "Expedition ended" :
      out->phase == PROBE_SENT ? "Expedition sent" : out->phase == PROBE_RETAINED ? "Retained expedition" :
      out->failed ? "Progress preserved" : "Choose an expedition";
  snprintf(out->title, sizeof(out->title), "%s", title);
  snprintf(out->status, sizeof(out->status), "%s", out->failed ? "Storage unavailable. Progress preserved." :
      out->phase == PROBE_SENT ? kit_stage(kit) : out->phase == PROBE_ENDED ?
      (transfer == KIT_COMPLETE ? "Cargo transferred / choose a new outing." : "Lab accepted / receipt pending.") :
      out->phase == PROBE_RETAINED ? "Saved outing / no retained field map" :
      live ? kit_route(kit) : "Choose where to explore");
  snprintf(out->context, sizeof(out->context), "%s", view->message[0] ? view->message :
      live && out->field.map.site_collected[4] ? "Sealed sample / contents unknown" :
      live && out->field.map.site_inspected[1] ? "Trace found / a known trail continues" :
      live ? "Preparation toward a supply attempt" : out->status);
  /* Presentation wording keeps the existing outcome/recovery meaning in one
   * 18px line; Kit's saved message and game command semantics stay unchanged. */
  if (!strcmp(view->message, "Sealed-container trail found. Route revealed."))
    strcpy(out->context, "Trace found / route revealed");
  else if (!strcmp(view->message, "Gathering source selected. Preparation kept."))
    strcpy(out->context, "Source selected / preparation kept");
  else if (!strcmp(view->message, "Sample store full or prototype sample limit reached. Cache kept."))
    strcpy(out->context, "Sample full / limit reached; cache kept");
  else if (!strcmp(view->message, "Prototype sample limit reached. Supplies remain available."))
    strcpy(out->context, "Sample limit / supplies still available");
  else if (!strcmp(view->message, "Source finished. Explore another opportunity."))
    strcpy(out->context, "Source finished / explore another place");
  snprintf(out->source, sizeof(out->source), "%s",
           saved_preparation ? "Preparation retained / no active source" : "No active source");
  if (live) {
    const char *names[] = {"Data", "Energy", "Essence"};
    for (unsigned resource = 0; resource < 3; ++resource) {
      unsigned state = out->field.preparation_status[resource];
      if (state == EXPEDITION_PREP_ACTIVE || state == EXPEDITION_PREP_CAPACITY_FULL) {
        snprintf(out->source, sizeof(out->source), "%s / %s / %u attempts left",
                 names[resource], out->field.source_name[resource], out->field.remaining_chances[resource]);
        break;
      }
    }
  }
  const char *preparation_states[] = {"Not started", "Active", "Paused", "Finished", "Hold full"};
  for (unsigned resource = 0; resource < 3; ++resource) {
    unsigned state = out->field.preparation_status[resource];
    if (state > EXPEDITION_PREP_CAPACITY_FULL) state = EXPEDITION_PREP_NOT_STARTED;
    snprintf(out->preparation_labels[resource], sizeof(out->preparation_labels[resource]), "%s",
             out->failed ? (live || out->field.preparation_ms[resource] ? "Saved" : "Unavailable") : preparation_states[state]);
  }
  if (out->failed) {
    snprintf(out->context, sizeof(out->context), "%s", out->status);
    snprintf(out->source, sizeof(out->source), "%s",
             live || saved_preparation ? "Saved preparation / actions unavailable" : "Preparation unavailable");
  }
  if (!selector && !out->failed && out->phase != PROBE_MAP) {
    out->action_count = kit_option_count(kit, KIT_COMPANION);
    if (out->action_count > 3) return 0;
    for (unsigned action = 0; action < out->action_count; ++action)
      snprintf(out->actions[action], sizeof(out->actions[action]), "%s", kit_option(kit, KIT_COMPANION, action));
  }
  snprintf(out->footer, sizeof(out->footer), "%s", selector ? "Left/Right: mode / Down/Confirm: enter" :
      out->phase == PROBE_MAP ? "Directions: move / Confirm: inspect / Back: modes" :
      out->phase == PROBE_SITE ? "Up/Down: choose / Confirm: act / Back: map" :
      "Up/Down: choose / Confirm: enter / Back: modes");
  return 1;
}
