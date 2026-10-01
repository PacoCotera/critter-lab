#include "probe_ui.h"
#include "field_art.h"
#include "ui_theme.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

enum { TILE_SIZE = 32, VIEW_WIDTH = 384, VIEW_HEIGHT = 320, EDGE_CUES = 44 };
struct NativeProbeUi {
  lv_obj_t *root, *viewport, *world, *title, *status, *context, *source, *footer;
  lv_obj_t *tiles[EXPEDITION_MAP_CELLS], *places[5], *player[2], *cues[EDGE_CUES];
  lv_obj_t *sheet, *buttons[3], *action_text[3], *rail_focus;
  lv_obj_t *quantities[3], *capsule, *capsule_text, *capsule_quantity;
  lv_obj_t *choice_image[3];
  NativeUiFrame focus, reached;
  NativeUiImage art[FIELD_ART_COUNT], materials[3];
  lv_point_precise_t player_points[9], cue_points[EDGE_CUES][3];
  CompanionProbeView view;
  char quantity_text[3][16], capsule_count[24];
};
static lv_obj_t *box(lv_obj_t *parent, int x, int y, int width, int height, uint32_t color) {
  lv_obj_t *object = lv_obj_create(parent);
  if (!object) return NULL;
  native_ui_surface(object, color, CORE_ART_BLUE_RGB, 0);
  lv_obj_set_pos(object, x, y);
  lv_obj_set_size(object, width, height);
  return object;
}
static lv_obj_t *text(lv_obj_t *parent, const lv_font_t *font, int x, int y,
                      int width, int height, const char *value) {
  lv_obj_t *object = lv_label_create(parent);
  if (!object) return NULL;
  native_ui_text(object, font, CORE_ART_INK_RGB);
  lv_obj_set_pos(object, x, y);
  lv_obj_set_size(object, width, height);
  lv_label_set_long_mode(object, LV_LABEL_LONG_MODE_WRAP);
  lv_label_set_text_static(object, value);
  return object;
}
static lv_obj_t *image(lv_obj_t *parent, const NativeUiImage *asset, int x, int y) {
  lv_obj_t *object = lv_image_create(parent);
  if (!object) return NULL;
  lv_obj_set_clickable(object, false);
  lv_image_set_src(object, &asset->image);
  lv_image_set_antialias(object, false);
  lv_obj_set_pos(object, x, y);
  return object;
}
static lv_obj_t *line(lv_obj_t *parent, const lv_point_precise_t *points,
                      unsigned count, uint32_t color, unsigned width) {
  lv_obj_t *object = lv_line_create(parent);
  if (!object) return NULL;
  lv_obj_remove_style_all(object);
  lv_obj_set_clickable(object, false);
  lv_obj_set_style_line_color(object, lv_color_hex(color), 0);
  lv_obj_set_style_line_width(object, width, 0);
  lv_obj_set_style_line_rounded(object, false, 0);
  lv_line_set_points(object, points, count);
  return object;
}
NativeProbeUi *native_probe_ui_create(lv_obj_t *parent, lv_group_t *group,
                                      const lv_font_t *body, const lv_font_t *place, const lv_font_t *small,
                                      const lv_font_t *action, const NativeUiImage *sample) {
  NativeProbeUi *ui = calloc(1, sizeof(*ui));
  if (!ui) return NULL;
  for (unsigned asset = 0; asset < FIELD_ART_COUNT; ++asset)
    if (!native_ui_image_from_sprite(&ui->art[asset], field_art_sprite((FieldArtId)asset))) goto failure;
  for (unsigned material = 0; material < 3; ++material)
    if (!native_ui_image_init(&ui->materials[material], (CoreArtId)(CORE_ART_DATA_COMPACT + material))) goto failure;
  ui->root = box(parent, 0, 0, 450, 600, CORE_ART_GRAPHITE_RGB);
  if (!ui->root) goto failure;
  const char *modes[] = {"Probe", "Cargo", "Companions"};
  for (unsigned mode = 0; mode < 3; ++mode) {
    lv_obj_t *name = text(ui->root, body, 29 + (int)mode * 126, 19, mode == 2 ? 142 : 110, 24, modes[mode]);
    if (!name) goto failure;
    if (mode) lv_obj_set_style_text_color(name, lv_color_hex(CORE_ART_SECONDARY_RGB), 0);
    else {
      lv_obj_set_style_border_side(name, LV_BORDER_SIDE_BOTTOM, 0);
      lv_obj_set_style_border_width(name, 2, 0);
      lv_obj_set_style_border_color(name, lv_color_hex(CORE_ART_BLUE_RGB), 0);
    }
  }
  ui->title = text(ui->root, place, 28, 58, 394, 34, "");
  ui->status = text(ui->root, small, 33, 112, 384, 60, "");
  ui->viewport = box(ui->root, 33, 98, VIEW_WIDTH, VIEW_HEIGHT, CORE_ART_SHADOW_RGB);
  if (!ui->viewport) goto failure;
  ui->world = box(ui->viewport, 0, 0, 640, 352, CORE_ART_SHADOW_RGB);
  if (!ui->world) goto failure;
  for (unsigned cell = 0; cell < EXPEDITION_MAP_CELLS; ++cell) {
    ui->tiles[cell] = image(ui->world, &ui->art[FIELD_ART_GRASS_A],
                            (cell % 20) * TILE_SIZE, (cell / 20) * TILE_SIZE);
    if (!ui->tiles[cell]) goto failure;
  }
  const FieldArtId place_art[] = {FIELD_ART_CAMP, FIELD_ART_MOSS_BEND,
                                  FIELD_ART_RELAY, FIELD_ART_STONE_SHELF, FIELD_ART_CACHE};
  for (unsigned place = 0; place < 5; ++place) {
    ui->places[place] = image(ui->world, &ui->art[place_art[place]], 0, 0);
    if (!ui->places[place]) goto failure;
  }
  if (!native_ui_frame_init(&ui->reached, ui->world, 36, 36, CORE_ART_BLUE_HIGHLIGHT_RGB)) goto failure;
  /* A small corner pin marks the occupied tile without covering its finding. */
  const lv_point_precise_t diamond[9] = {{8,4},{10,6},{12,8},{10,10},{8,12},{6,10},{4,8},{6,6},{8,4}};
  memcpy(ui->player_points, diamond, sizeof(diamond));
  ui->player[0] = line(ui->world, ui->player_points, 9, CORE_ART_BLUE_RGB, 4);
  ui->player[1] = line(ui->world, ui->player_points, 9, CORE_ART_INK_RGB, 2);
  if (!ui->player[0] || !ui->player[1]) goto failure;
  for (unsigned cue = 0; cue < EDGE_CUES; ++cue) {
    ui->cues[cue] = line(ui->viewport, ui->cue_points[cue], 3, CORE_ART_BLUE_HIGHLIGHT_RGB, 2);
    if (!ui->cues[cue]) goto failure;
  }
  ui->sheet = box(ui->root, 33, 302, 384, 116, CORE_ART_SHADOW_RGB);
  if (!ui->sheet) goto failure;
  for (unsigned choice = 0; choice < 3; ++choice) {
    ui->buttons[choice] = lv_button_create(ui->sheet);
    if (!ui->buttons[choice]) goto failure;
    native_ui_action(ui->buttons[choice]);
    lv_obj_set_pos(ui->buttons[choice], choice * 128, 0);
    lv_obj_set_size(ui->buttons[choice], 128, 116);
    lv_group_add_obj(group, ui->buttons[choice]);
    ui->action_text[choice] = text(ui->buttons[choice], small, 8, 68, 112, 44, "");
    ui->choice_image[choice] = image(ui->buttons[choice], &ui->materials[choice], 40, 8);
    if (!ui->action_text[choice] || !ui->choice_image[choice]) goto failure;
  }
  for (unsigned material = 0; material < 3; ++material) {
    const CoreArtSprite *sprite = core_art_sprite((CoreArtId)(CORE_ART_DATA_COMPACT + material));
    int left = 28 + (int)material * 88;
    if (!image(ui->root, &ui->materials[material], left, 550 - sprite->center_y)) goto failure;
    ui->quantities[material] = text(ui->root, action, left + sprite->width + 6, 539, 34, 27, "0");
    if (!ui->quantities[material]) goto failure;
  }
  ui->capsule = image(ui->root, sample, 296, 524);
  ui->capsule_text = text(ui->root, small, 357, 528, 64, 22, "Sample");
  ui->capsule_quantity = text(ui->root, action, 357, 551, 64, 27, "0 / 1");
  ui->context = text(ui->root, body, 33, 436, 384, 24, "");
  ui->source = text(ui->root, small, 33, 461, 384, 22, "");
  ui->footer = text(ui->root, small, 33, 486, 384, 22, "");
  if (!native_ui_frame_init(&ui->focus, ui->root, 128, 116, CORE_ART_FOCUS_RGB)) goto failure;
  ui->rail_focus = box(ui->root, 23, 14, 111, 31, CORE_ART_SHADOW_RGB);
  if (!ui->rail_focus) goto failure;
  lv_obj_set_style_bg_opa(ui->rail_focus, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(ui->rail_focus, 2, 0);
  lv_obj_set_style_border_color(ui->rail_focus, lv_color_hex(CORE_ART_FOCUS_RGB), 0);
  if (!ui->title || !ui->status || !ui->capsule || !ui->capsule_text ||
      !ui->capsule_quantity || !ui->context || !ui->source || !ui->footer) goto failure;
  lv_obj_set_hidden(ui->root, true);
  return ui;
failure:
  native_probe_ui_destroy(ui);
  return NULL;
}
void native_probe_ui_destroy(NativeProbeUi *ui) {
  if (!ui) return;
  if (ui->root) lv_obj_delete(ui->root);
  for (unsigned asset = 0; asset < FIELD_ART_COUNT; ++asset) native_ui_image_destroy(&ui->art[asset]);
  for (unsigned material = 0; material < 3; ++material) native_ui_image_destroy(&ui->materials[material]);
  free(ui);
}
void native_probe_ui_hide(NativeProbeUi *ui) {
  if (ui) lv_obj_set_hidden(ui->root, true);
}
static FieldArtId tile_art(const ExpeditionMapView *map, unsigned cell) {
  if (map->paths[cell]) {
    unsigned mask = probe_path_neighbors(map, cell);
#if defined(FIELD_ART_HAS_BRIDGE) && FIELD_ART_HAS_BRIDGE
    if (map->terrain[cell] == EXPEDITION_TERRAIN_WATER) {
      if (mask == 10) return FIELD_ART_BRIDGE_EW;
      if (mask == 5) return FIELD_ART_BRIDGE_NS;
    }
#endif
    return (FieldArtId)(FIELD_ART_PATH_0 + mask);
  }
  switch (map->terrain[cell]) {
  case EXPEDITION_TERRAIN_TREE: return FIELD_ART_TREE;
  case EXPEDITION_TERRAIN_STONE: return FIELD_ART_BOULDER;
  case EXPEDITION_TERRAIN_WATER: return FIELD_ART_WATER;
  case EXPEDITION_TERRAIN_GRASS: return FIELD_ART_GRASS_A;
  default: return FIELD_ART_GRASS_A;
  }
}
static void edge_cue(NativeProbeUi *ui, unsigned *count, unsigned side, int at, int height) {
  if (*count >= EDGE_CUES) return;
  unsigned index = (*count)++;
  const lv_point_precise_t shapes[4][3] = {
    {{5,0},{0,4},{5,8}},{{0,0},{5,4},{0,8}},{{0,5},{4,0},{8,5}},{{0,0},{4,5},{8,0}}
  };
  memcpy(ui->cue_points[index], shapes[side], sizeof(ui->cue_points[index]));
  lv_line_set_points(ui->cues[index], ui->cue_points[index], 3);
  lv_obj_set_pos(ui->cues[index], side < 2 ? (side ? VIEW_WIDTH - 6 : 1) : at - 4,
                 side < 2 ? at - 4 : (side == 2 ? 1 : height - 6));
  lv_obj_set_hidden(ui->cues[index], false);
}
static void show_edge_cues(NativeProbeUi *ui, int camera_x, int camera_y, int height) {
  const ExpeditionMapView *map = &ui->view.field.map;
  unsigned count = 0;
  for (unsigned cell = 0; cell < EXPEDITION_MAP_CELLS; ++cell) {
    if (!map->paths[cell]) continue;
    int x = (cell % 20) * 32 + 16, y = (cell / 20) * 32 + 16;
    unsigned neighbors = probe_path_neighbors(map, cell);
    if ((neighbors & 2) && y > camera_y + 8 && y < camera_y + height - 8) {
      if (camera_x > 0 && x <= camera_x && x + 32 > camera_x) edge_cue(ui, &count, 0, y - camera_y, height);
      if (camera_x + VIEW_WIDTH < 640 && x < camera_x + VIEW_WIDTH && x + 32 >= camera_x + VIEW_WIDTH)
        edge_cue(ui, &count, 1, y - camera_y, height);
    }
    if ((neighbors & 4) && x > camera_x + 8 && x < camera_x + VIEW_WIDTH - 8) {
      if (camera_y > 0 && y <= camera_y && y + 32 > camera_y) edge_cue(ui, &count, 2, x - camera_x, height);
      if (camera_y + height < 352 && y < camera_y + height && y + 32 >= camera_y + height)
        edge_cue(ui, &count, 3, x - camera_x, height);
    }
  }
  for (; count < EDGE_CUES; ++count) lv_obj_set_hidden(ui->cues[count], true);
}
int native_probe_ui_update(NativeProbeUi *ui, const CompanionProbeView *view) {
  if (!ui || !view || view->action_count > 3) return 0;
  for (unsigned choice = 0; choice < 3; ++choice)
    if (view->choice_material[choice] > 3) return 0;
  ui->view = *view;
  view = &ui->view;
  lv_obj_set_hidden(ui->root, false);
  lv_label_set_text_static(ui->title, view->title);
  lv_label_set_text_static(ui->status, view->status);
  lv_label_set_text_static(ui->context, !strcmp(view->context, view->status) &&
      view->phase == PROBE_ENTRY ? "" : view->context);
  lv_label_set_text_static(ui->source, view->source);
  lv_label_set_text_static(ui->footer, "");
  lv_obj_set_hidden(ui->footer, true);
  int scene = view->phase == PROBE_MAP || view->phase == PROBE_SITE;
  lv_obj_set_hidden(ui->viewport, !scene);
  lv_obj_set_hidden(ui->status, scene);
  lv_obj_set_hidden(ui->rail_focus, !view->selector);
  int scene_height = VIEW_HEIGHT;
  if (view->phase == PROBE_SITE) scene_height = 192;
  lv_obj_set_height(ui->viewport, scene_height);
  int sheet_y = 302;
  lv_obj_set_pos(ui->sheet, 33, sheet_y);
  lv_obj_set_height(ui->sheet, 116);
  lv_obj_set_hidden(ui->sheet, view->selector || !view->action_count);
  for (unsigned choice = 0; choice < 3; ++choice) {
    lv_obj_set_hidden(ui->buttons[choice], choice >= view->action_count);
    lv_obj_remove_state(ui->buttons[choice], LV_STATE_FOCUSED | LV_STATE_PRESSED);
    if (choice < view->action_count) lv_label_set_text_static(ui->action_text[choice], view->actions[choice]);
    unsigned material_id = view->choice_material[choice];
    int material = view->phase == PROBE_SITE && material_id > 0 && material_id <= 3;
    lv_obj_set_hidden(ui->choice_image[choice], !material);
    if (material) {
      unsigned resource = material_id - 1;
      lv_image_set_src(ui->choice_image[choice], &ui->materials[resource].image);
    }
  }
  int focus = !view->selector && view->focus < view->action_count;
  lv_obj_set_hidden(ui->focus.object, !focus);
  if (focus) {
    lv_group_focus_obj(ui->buttons[view->focus]);
    lv_obj_add_state(ui->buttons[view->focus], LV_STATE_FOCUSED);
    if (view->pressed) lv_obj_add_state(ui->buttons[view->focus], LV_STATE_PRESSED);
    lv_obj_set_pos(ui->focus.object, 33 + (int)view->focus * 128, sheet_y);
  }
  if (scene) {
    const ExpeditionMapView *map = &view->field.map;
    int camera_x, camera_y;
    probe_camera(map, VIEW_WIDTH, scene_height, &camera_x, &camera_y);
    lv_obj_set_pos(ui->world, -camera_x, -camera_y);
    for (unsigned cell = 0; cell < EXPEDITION_MAP_CELLS; ++cell)
      lv_image_set_src(ui->tiles[cell], &ui->art[tile_art(map, cell)].image);
    for (unsigned place = 0; place < 5; ++place) {
      lv_obj_set_hidden(ui->places[place], !map->site_visible[place]);
      lv_obj_set_pos(ui->places[place], map->site_x[place] * 32, map->site_y[place] * 32);
    }
    for (unsigned stroke = 0; stroke < 2; ++stroke) {
      lv_obj_set_hidden(ui->player[stroke], !map->avatar_visible);
      lv_obj_set_pos(ui->player[stroke], map->avatar_x * 32, map->avatar_y * 32);
    }
    unsigned site = view->field.current_site;
    lv_obj_set_hidden(ui->reached.object, site >= 5 || !map->site_visible[site]);
    if (site < 5) lv_obj_set_pos(ui->reached.object, map->site_x[site] * 32 - 2, map->site_y[site] * 32 - 2);
    show_edge_cues(ui, camera_x, camera_y, scene_height);
  }
  for (unsigned material = 0; material < 3; ++material) {
    snprintf(ui->quantity_text[material], sizeof(ui->quantity_text[material]), "%" PRIu32, view->cargo.supplies[material]);
    lv_label_set_text_static(ui->quantities[material], ui->quantity_text[material]);
  }
  lv_obj_set_hidden(ui->capsule, !view->cargo.capsules);
  snprintf(ui->capsule_count, sizeof(ui->capsule_count), "%" PRIu32 " / %" PRIu32, view->cargo.capsules, view->cargo.capsule_capacity);
  lv_label_set_text_static(ui->capsule_quantity, ui->capsule_count);
  return 1;
}
