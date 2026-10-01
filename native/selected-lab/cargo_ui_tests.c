#define _POSIX_C_SOURCE 200809L
#include "native_ui.h"
#include "ui_assets.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void image_fidelity(void) {
  const CoreArtId ids[] = {CORE_ART_DATA_PRIMARY, CORE_ART_ENERGY_PRIMARY,
                           CORE_ART_ESSENCE_PRIMARY, CORE_ART_SAMPLE_NEUTRAL};
  for (unsigned index = 0; index < 4; ++index) {
    NativeUiImage converted;
    const CoreArtSprite *source = core_art_sprite(ids[index]);
    assert(native_ui_image_init(&converted, ids[index]));
    assert(converted.image.header.w == source->width && converted.image.header.h == source->height);
    for (unsigned pixel = 0; pixel < source->width * source->height; ++pixel) {
      assert(converted.pixels[pixel * 4] == source->rgba[pixel * 4 + 2]);
      assert(converted.pixels[pixel * 4 + 1] == source->rgba[pixel * 4 + 1]);
      assert(converted.pixels[pixel * 4 + 2] == source->rgba[pixel * 4]);
      assert(converted.pixels[pixel * 4 + 3] == source->rgba[pixel * 4 + 3]);
    }
    native_ui_image_destroy(&converted);
  }
}
static void projection_truth(void) {
  SelectedLab lab;
  selected_lab_init(&lab);
  DeviceKit kit = {0};
  kit.lab = &lab;
  kit.companion.page = COMP_CARGO;
  kit.companion.mode = COMP_CARGO;
  kit.journal.phase = KIT_WAITING;
  kit.sealed_field.version = 1;
  strcpy(kit.sealed_field.expedition_id, "cargo-proof");
  kit.sealed_field.cargo[0] = 7 * GAME_SUPPLY_UNIT;
  kit.sealed_field.collected = 1;
  strcpy(kit.journal.haul_id, "cargo-proof");
  CompanionCargoView sent;
  assert(kit_cargo_projection(&kit, &sent));
  assert(sent.supplies[0] == 7 && sent.capsules == 1 && !sent.accepted);
  /* The immutable received record proves acceptance even before ack completion. */
  lab.game.received_count = 1;
  lab.game.received[0] = kit.sealed_field;
  kit.journal.phase = KIT_ACK_PENDING;
  CompanionCargoView accepted;
  assert(kit_cargo_projection(&kit, &accepted));
  assert(accepted.accepted && !accepted.supplies[0] && !accepted.capsules);
  assert(accepted.delivered[0] == 7 && accepted.delivered_capsules == 1);
  assert(!strcmp(accepted.title, "Cargo empty"));
  CompanionCargoView owned = accepted;
  memset(&kit, 0, sizeof(kit));
  assert(!memcmp(&owned, &accepted, sizeof(owned)));
}
static CompanionCargoView example(void) {
  CompanionCargoView view = {0};
  strcpy(view.identity, "render-proof");
  strcpy(view.title, "Cargo");
  strcpy(view.context, "Field survey");
  strcpy(view.capsule, "1 sealed sample");
  strcpy(view.detail, "Contents unknown");
  strcpy(view.capacity, "Supplies 40 / 40   Capsules 1 / 1");
  strcpy(view.feedback, "Lab link available");
  strcpy(view.actions[0], "Send to Lab");
  strcpy(view.actions[1], "Discard items");
  view.action_count = 2;
  view.supplies[0] = 12;
  view.supplies[1] = 15;
  view.supplies[2] = 13;
  view.capsules = view.capsule_capacity = 1;
  return view;
}
static void export_bmp(const char *path, const uint8_t *rgb) {
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
static void rendering_and_motion(const char *directory) {
  NativeUiContext *context = native_ui_create();
  NativeUiContext *second = native_ui_create();
  assert(context && second);
  CompanionCargoView view = example(), untouched = view;
  const uint8_t *rgb = native_ui_cargo(context, &view, 1);
  assert(rgb && !memcmp(&view, &untouched, sizeof(view)));
  assert(rgb[0] == 0x1e && rgb[1] == 0x28 && rgb[2] == 0x2f);
  uint8_t *still = malloc(450 * 600 * 3);
  assert(still);
  memcpy(still, rgb, 450 * 600 * 3);
  assert(native_ui_cargo(second, &view, 1));
  native_ui_destroy(second);
  assert(!memcmp(still, native_ui_cargo(context, &view, 1), 450 * 600 * 3));
  view.focus = 1;
  rgb = native_ui_cargo(context, &view, 0);
  assert(rgb && native_ui_animation_pending(context));
  if (directory) {
    char path[512];
    snprintf(path, sizeof(path), "%s/cargo-still.bmp", directory);
    export_bmp(path, still);
    snprintf(path, sizeof(path), "%s/cargo-focus-0.bmp", directory);
    export_bmp(path, rgb);
  }
  view.held = 1;
  assert(native_ui_cargo(context, &view, 0));
  native_ui_advance(context, 120);
  assert(native_ui_animation_pending(context));
  view.held = 0;
  assert(native_ui_cargo(context, &view, 0));
  native_ui_advance(context, 60);
  rgb = native_ui_cargo(context, &view, 0);
  assert(rgb && native_ui_animation_pending(context));
  if (directory) {
    char path[512];
    snprintf(path, sizeof(path), "%s/cargo-focus-60.bmp", directory);
    export_bmp(path, rgb);
  }
  native_ui_advance(context, 60);
  rgb = native_ui_cargo(context, &view, 0);
  assert(rgb && !native_ui_animation_pending(context));
  if (directory) {
    char path[512];
    snprintf(path, sizeof(path), "%s/cargo-focus-120.bmp", directory);
    export_bmp(path, rgb);
  }
  view.focus = 0;
  assert(native_ui_cargo(context, &view, 0) && native_ui_animation_pending(context));
  view.suspended = 1;
  assert(native_ui_cargo(context, &view, 0) && !native_ui_animation_pending(context));
  native_ui_destroy(context);
  /* A new context must survive full LVGL teardown and have no prior Kit identity. */
  context = native_ui_create();
  assert(context && native_ui_cargo(context, &untouched, 1));
  native_ui_destroy(context);
  free(still);
}
int main(int argc, char **argv) {
  image_fidelity();
  projection_truth();
  rendering_and_motion(argc == 2 ? argv[1] : NULL);
  puts("Companion Cargo UI checks passed");
  return 0;
}
