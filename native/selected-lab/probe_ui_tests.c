#include "native_ui.h"
#include "field_art.h"
#include "ui_assets.h"
#include "expedition.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void field_fixture(SelectedLab *lab, DeviceKit *kit) {
  selected_lab_init(lab);
  memset(kit, 0, sizeof(*kit));
  kit->lab = lab;
  kit->companion.page = COMP_PROBE;
  kit->journal.companion_online = 1;
  game_rules_resume_runtime(&lab->game, 100);
  GameCommand start = {0};
  start.type = GAME_COMMAND_FIELD_START;
  start.data.field.seed = 67891;
  start.data.field.sample_budget = 1;
  start.data.field.monotonic_seconds = 100;
  assert(game_field_start(&lab->game, &start) == GAME_OK);
}
static void projection_and_phase_guards(void) {
  SelectedLab lab;
  DeviceKit kit;
  field_fixture(&lab, &kit);
  CompanionProbeView before, after;
  GameState unchanged = lab.game;
  assert(kit_probe_projection(&kit, &before));
  assert(before.phase == PROBE_MAP && !before.action_count && !before.field.map.site_visible[4]);
  for (unsigned cell = 0; cell < GAME_FIELD_CELLS; ++cell)
    assert(before.field.map.paths[cell] == lab.game.field.paths[cell]);
  assert(!memcmp(&unchanged, &lab.game, sizeof(unchanged)));
  lab.game.field.trace = 1;
  assert(kit_probe_projection(&kit, &after) && after.field.map.site_visible[4]);
  for (unsigned cell = 0; cell < GAME_FIELD_CELLS; ++cell)
    assert(after.field.map.paths[cell] == !!(lab.game.field.paths[cell] || lab.game.field.hidden_paths[cell]));
  kit.companion.page = COMP_FIELD_SITE;
  kit.companion.focus = 2;
  assert(kit_probe_projection(&kit, &after));
  assert(after.phase == PROBE_SITE && after.action_count == 3 && after.focus == 2);
  assert(!strcmp(after.actions[2], "Gather Essence"));
  lab.game.field.x = lab.game.field.site_x[4];
  lab.game.field.y = lab.game.field.site_y[4];
  lab.game.field.trace = 1;
  lab.game.field.collected = 1;
  assert(kit_probe_projection(&kit, &after));
  assert(after.phase == PROBE_SITE && !after.action_count);
  assert(!strcmp(after.footer, "Back: map"));
  lab.game = unchanged;
  CompanionCargoView cargo;
  assert(!kit_cargo_projection(&kit, &cargo));
  CompanionCargoFacts facts;
  assert(kit_cargo_facts(&kit, &facts));
  kit.companion.page = COMP_MODES;
  kit.companion.mode = COMP_PROBE;
  assert(kit_probe_projection(&kit, &after) && after.selector && !after.action_count);
  assert(strstr(after.footer, "mode") && !strstr(after.footer, "inspect"));
  kit.companion.mode = COMP_CARGO;
  assert(!kit_probe_projection(&kit, &after));
  kit.companion.page = COMP_PROBE;
  lab.game.field.active_source = 0;
  lab.game.field.last_source[0] = 0;
  lab.game.gather_progress_ms[0] = 1500;
  kit.failed = 1;
  assert(kit_probe_projection(&kit, &after) && after.phase == PROBE_UNAVAILABLE && !after.action_count);
  assert(after.preparation_available && after.field.preparation_ms[0] == 1500);
  assert(!strcmp(after.preparation_labels[0], "Saved") && !strstr(after.source, "Active"));
  assert(!strcmp(after.source, "Saved preparation / actions unavailable"));
  GameState retained = lab.game;
  memset(&lab.game.field, 0, sizeof(lab.game.field));
  lab.game.expedition_id[0] = 0;
  lab.game.gather_progress_ms[0] = 0;
  assert(kit_probe_projection(&kit, &after) && !after.preparation_available);
  assert(!strcmp(after.preparation_labels[0], "Unavailable"));
  assert(!strcmp(after.source, "Preparation unavailable"));
  lab.game = retained;
  kit.failed = 0;
  game_field_record(&lab.game, &kit.sealed_field);
  kit.journal.phase = KIT_WAITING;
  assert(kit_probe_projection(&kit, &after) && after.phase == PROBE_SENT);
  assert(after.preparation_available && after.field.preparation_ms[0] == 1500);
  assert(!strcmp(after.preparation_labels[0], "Paused"));
  kit.journal.phase = KIT_ACK_PENDING;
  memset(&lab.game.field, 0, sizeof(lab.game.field));
  lab.game.expedition_id[0] = 0;
  lab.game.expedition_data = lab.game.expedition_energy = lab.game.expedition_essence = 0;
  assert(kit_probe_projection(&kit, &after) && after.phase == PROBE_ENDED);
  assert(after.preparation_available && after.field.preparation_ms[0] == 1500);
  assert(!strcmp(after.preparation_labels[0], "Paused"));
  assert(!after.cargo.supplies[0] && !after.cargo.capsules && !after.field.map.avatar_visible);
  kit.journal.phase = KIT_COMPLETE;
  assert(kit_probe_projection(&kit, &after) && after.action_count == 3);
  kit.journal.phase = KIT_IDLE;
  memset(&kit.sealed_field, 0, sizeof(kit.sealed_field));
  assert(kit_probe_projection(&kit, &after) && after.phase == PROBE_ENTRY);
  assert(after.field.preparation_ms[0] == 1500 && !strcmp(after.preparation_labels[0], "Paused"));
  strcpy(lab.game.expedition_id, "legacy-outing");
  assert(kit_probe_projection(&kit, &after) && after.phase == PROBE_RETAINED);
  assert(after.field.preparation_ms[0] == 1500 && !strcmp(after.preparation_labels[0], "Paused"));
  /* A new outing has no source selected yet; retained preparation is paused. */
  lab.game.expedition_id[0] = 0;
  GameCommand next = {0};
  next.type = GAME_COMMAND_FIELD_START;
  next.data.field.seed = 777;
  next.data.field.monotonic_seconds = 100;
  assert(game_field_start(&lab.game, &next) == GAME_OK);
  assert(kit_probe_projection(&kit, &after) && after.phase == PROBE_MAP);
  assert(after.field.preparation_ms[0] == 1500 && !strcmp(after.preparation_labels[0], "Paused"));
}
static void legal_neighbors_and_camera(void) {
  ExpeditionMapView map = {0};
  map.paths[19] = map.paths[20] = 1;
  assert(!probe_path_neighbors(&map, 19) && !probe_path_neighbors(&map, 20));
  map.paths[0] = map.paths[1] = 1;
  assert(probe_path_neighbors(&map, 0) == 6); /* South20 and East1; never West19. */
  int x, y;
  map.avatar_x = map.avatar_y = 0;
  probe_camera(&map, 384, 288, &x, &y);
  assert(x == 0 && y == 0);
  map.avatar_x = 19; map.avatar_y = 10;
  probe_camera(&map, 384, 288, &x, &y);
  assert(x == 256 && y == 64);
  probe_camera(&map, 384, 186, &x, &y);
  assert(x == 256 && y == 166);
  map.avatar_x = 10; map.avatar_y = 5;
  probe_camera(&map, 384, 288, &x, &y);
  assert(x == 144 && y == 32);
}
static void asset_fidelity(void) {
  for (unsigned asset = 0; asset < FIELD_ART_COUNT; ++asset) {
    const CoreArtSprite *source = field_art_sprite((FieldArtId)asset);
    assert(source && source->width == 32 && source->height == 32);
    NativeUiImage image;
    assert(native_ui_image_from_sprite(&image, source));
    for (unsigned pixel = 0; pixel < 32 * 32; ++pixel) {
      assert(image.pixels[pixel * 4] == source->rgba[pixel * 4 + 2]);
      assert(image.pixels[pixel * 4 + 1] == source->rgba[pixel * 4 + 1]);
      assert(image.pixels[pixel * 4 + 2] == source->rgba[pixel * 4]);
      assert(image.pixels[pixel * 4 + 3] == source->rgba[pixel * 4 + 3]);
    }
    native_ui_image_destroy(&image);
  }
}
static void retained_roots_and_memory(void) {
  SelectedLab lab;
  DeviceKit kit;
  field_fixture(&lab, &kit);
  NativeUiContext *context = native_ui_create();
  assert(context);
  CompanionProbeView probe;
  assert(kit_probe_projection(&kit, &probe));
  GameState before = lab.game;
  const uint8_t *rgb = native_ui_probe(context, &probe);
  assert(rgb && !memcmp(&before, &lab.game, sizeof(before)));
  lv_mem_monitor_t initial, final;
  lv_mem_monitor(&initial);
  kit.companion.page = COMP_CARGO;
  CompanionCargoView cargo;
  assert(kit_cargo_projection(&kit, &cargo));
  uint8_t *cargo_pixels = malloc(450 * 600 * 3);
  assert(cargo_pixels);
  rgb = native_ui_cargo(context, &cargo, 1);
  assert(rgb);
  /* Switching equality alone can accept two identically clipped frames.
   * Retained Cargo must also paint its frame outside LVGL's default130px root. */
  const unsigned frame_points[][2] = {{225,14},{225,585}};
  for (unsigned point = 0; point < 2; ++point) {
    unsigned pixel = (frame_points[point][1] * 450 + frame_points[point][0]) * 3;
    assert(rgb[pixel] != rgb[0] || rgb[pixel + 1] != rgb[1] || rgb[pixel + 2] != rgb[2]);
  }
  memcpy(cargo_pixels, rgb, 450 * 600 * 3);
  kit.companion.page = COMP_FIELD_SITE;
  assert(kit_probe_projection(&kit, &probe));
  assert(native_ui_probe(context, &probe));
  for (unsigned cycle = 0; cycle < 20; ++cycle) {
    assert(native_ui_probe(context, &probe));
    assert(!memcmp(cargo_pixels, native_ui_cargo(context, &cargo, 1), 450 * 600 * 3));
  }
  lv_mem_monitor(&final);
  printf("Probe/Cargo pool: %zu used, %zu max, %zu free of %zu bytes\n",
         final.total_size - final.free_size, final.max_used, final.free_size, final.total_size);
  assert(final.free_size >= 16384);
  /* Warmup adds finite state styles; repeated transitions must not accumulate. */
  size_t free_after_warmup = final.free_size;
  for (unsigned cycle = 0; cycle < 20; ++cycle) {
    assert(native_ui_probe(context, &probe));
    assert(native_ui_cargo(context, &cargo, 1));
  }
  lv_mem_monitor(&final);
  assert(final.free_size == free_after_warmup);
  NativeUiContext *second = native_ui_create();
  assert(second && native_ui_probe(second, &probe));
  lv_mem_monitor(&final);
  printf("Two retained UI contexts: %zu used / %zu bytes\n", final.total_size - final.free_size, final.total_size);
  native_ui_destroy(second);
  native_ui_destroy(context);
  free(cargo_pixels);
}
static void export_fixture(NativeUiContext *context, const char *directory,
                           const char *name, const CompanionProbeView *view) {
  CompanionProbeView before = *view;
  const uint8_t *rgb = native_ui_probe(context, view);
  assert(rgb && !memcmp(&before, view, sizeof(before)));
  char path[512];
  int length = snprintf(path, sizeof(path), "%s/%s.bmp", directory, name);
  assert(length > 0 && (size_t)length < sizeof(path));
  FILE *output = fopen(path, "wb");
  assert(output);
  unsigned stride = (450 * 3 + 3) & ~3u;
  unsigned fields[][2] = {{54 + stride * 600,4},{0,4},{54,4},{40,4},{450,4},
                          {600,4},{1,2},{24,2},{0,4},{stride * 600,4},
                          {2835,4},{2835,4},{0,4},{0,4}};
  assert(fwrite("BM", 1, 2, output) == 2);
  for (unsigned field = 0; field < sizeof(fields)/sizeof(fields[0]); ++field)
    for (unsigned byte = 0; byte < fields[field][1]; ++byte)
      assert(fputc((int)((fields[field][0] >> (8 * byte)) & 255), output) != EOF);
  uint8_t row[1352];
  for (unsigned y = 600; y > 0; --y) {
    memset(row, 0, sizeof(row));
    for (unsigned x = 0; x < 450; ++x)
      for (unsigned channel = 0; channel < 3; ++channel)
        row[x * 3 + channel] = rgb[((y - 1) * 450 + x) * 3 + 2 - channel];
    assert(fwrite(row, 1, stride, output) == stride);
  }
  assert(!fclose(output));
}
static void representative_exports(const char *directory) {
  SelectedLab lab;
  DeviceKit kit;
  field_fixture(&lab, &kit);
  NativeUiContext *context = native_ui_create();
  assert(context);
  CompanionProbeView map;
  assert(kit_probe_projection(&kit, &map));
  const unsigned corners[][2] = {{0,0},{19,0},{0,10},{19,10}};
  const char *names[] = {"fixture-camera-nw", "fixture-camera-ne", "fixture-camera-sw", "fixture-camera-se"};
  for (unsigned corner = 0; corner < 4; ++corner) {
    CompanionProbeView view = map;
    view.field.map.avatar_x = corners[corner][0];
    view.field.map.avatar_y = corners[corner][1];
    export_fixture(context, directory, names[corner], &view);
  }
  kit.companion.page = COMP_FIELD_SITE;
  kit.companion.focus = 2;
  CompanionProbeView site;
  assert(kit_probe_projection(&kit, &site) && site.action_count == 3);
  export_fixture(context, directory, "fixture-camp-three-actions", &site);
  CompanionProbeView full = map;
  full.cargo.supplies[0] = 40;
  full.cargo.supplies[1] = full.cargo.supplies[2] = 0;
  full.cargo.capsules = 1;
  full.field.preparation_status[0] = EXPEDITION_PREP_CAPACITY_FULL;
  full.field.preparation_ms[0] = GAME_GATHER_ATTEMPT_MS;
  strcpy(full.preparation_labels[0], "Hold full");
  strcpy(full.context, "Supply bag full / preparation kept");
  strcpy(full.source, "Data / Camp / 3 attempts left");
  export_fixture(context, directory, "fixture-hold-full40", &full);
  kit.companion.page = COMP_PROBE;
  lab.game.field.active_source = 0;
  lab.game.field.last_source[0] = 0;
  lab.game.gather_progress_ms[0] = 1500;
  kit.failed = 1;
  CompanionProbeView failed;
  assert(kit_probe_projection(&kit, &failed));
  assert(failed.preparation_available && !strcmp(failed.preparation_labels[0], "Saved"));
  export_fixture(context, directory, "fixture-unavailable-saved-preparation", &failed);
  native_ui_destroy(context);
  char path[512];
  int length = snprintf(path, sizeof(path), "%s/probe-fixtures.txt", directory);
  assert(length > 0 && (size_t)length < sizeof(path));
  FILE *metadata = fopen(path, "w");
  assert(metadata);
  fputs("Representative presentation fixtures, not actual play or saved-world claims.\n"
        "Camera fixtures relocate a copied avatar to each world corner; they do not claim legal movement there.\n"
        "Camp fixture has three real Kit choices and focus2.\n"
        "Hold-full fixture presents Data40, capsule1 and a full retained attempt.\n"
        "Unavailable fixture presents1500ms saved Data preparation, no map or actions.\n", metadata);
  assert(!fclose(metadata));
}
int main(int argc, char **argv) {
  projection_and_phase_guards();
  legal_neighbors_and_camera();
  asset_fidelity();
  retained_roots_and_memory();
  if (argc == 2) representative_exports(argv[1]);
  puts("Companion Probe retained UI checks passed");
  return 0;
}
