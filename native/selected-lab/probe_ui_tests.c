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
  kit.failed = 1;
  assert(kit_probe_projection(&kit, &after) && after.phase == PROBE_UNAVAILABLE && !after.action_count);
  kit.failed = 0;
  game_field_record(&lab.game, &kit.sealed_field);
  kit.journal.phase = KIT_WAITING;
  assert(kit_probe_projection(&kit, &after) && after.phase == PROBE_SENT);
  kit.journal.phase = KIT_ACK_PENDING;
  memset(&lab.game.field, 0, sizeof(lab.game.field));
  lab.game.expedition_id[0] = 0;
  lab.game.expedition_data = lab.game.expedition_energy = lab.game.expedition_essence = 0;
  assert(kit_probe_projection(&kit, &after) && after.phase == PROBE_ENDED);
  assert(!after.cargo.supplies[0] && !after.cargo.capsules && !after.field.map.avatar_visible);
  kit.journal.phase = KIT_COMPLETE;
  assert(kit_probe_projection(&kit, &after) && after.action_count == 3);
  kit.journal.phase = KIT_IDLE;
  memset(&kit.sealed_field, 0, sizeof(kit.sealed_field));
  strcpy(lab.game.expedition_id, "legacy-outing");
  assert(kit_probe_projection(&kit, &after) && after.phase == PROBE_RETAINED);
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
int main(void) {
  projection_and_phase_guards();
  legal_neighbors_and_camera();
  asset_fidelity();
  retained_roots_and_memory();
  puts("Companion Probe retained UI checks passed");
  return 0;
}
