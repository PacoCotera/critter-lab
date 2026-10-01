#define _POSIX_C_SOURCE 200809L
#include "native_ui.h"
#include "ui_assets.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

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
  /* Legacy accepted receipts retain journal evidence and actual sample origin,
   * while the now-empty carried inventory remains zero. */
  kit.lab = &lab;
  kit.companion.page = COMP_CARGO;
  kit.journal.phase = KIT_ACK_PENDING;
  kit.journal.cargo[1] = 9 * GAME_SUPPLY_UNIT;
  strcpy(kit.journal.haul_id, "legacy-proof");
  lab.game.sample_count = 1;
  strcpy(lab.game.samples[0].origin_expedition_id, "legacy-proof");
  assert(kit_cargo_projection(&kit, &accepted));
  assert(accepted.accepted && !accepted.supplies[1] && !accepted.capsules);
  assert(accepted.delivered[1] == 9 && accepted.delivered_capsules == 1);
  strcpy(lab.game.samples[0].origin_expedition_id, "different-outing");
  assert(kit_cargo_projection(&kit, &accepted) && !accepted.delivered_capsules);
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
static void rendering_and_motion(const char *directory, const CompanionCargoView *actual) {
  NativeUiContext *context = native_ui_create();
  NativeUiContext *second = native_ui_create();
  assert(context && second);
  CompanionCargoView view = actual ? *actual : example();
  assert(view.action_count == 2);
  view.focus = 0;
  view.held = view.pressed = view.suspended = 0;
  CompanionCargoView untouched = view;
  const uint8_t *rgb = native_ui_cargo(context, &view, 1);
  assert(rgb && !memcmp(&view, &untouched, sizeof(view)));
  assert(rgb[0] == 0x1e && rgb[1] == 0x28 && rgb[2] == 0x2f);
  uint8_t *still = malloc(450 * 600 * 3);
  assert(still);
  memcpy(still, rgb, 450 * 600 * 3);
  assert(native_ui_cargo(second, &view, 1));
  view.focus = 1;
  assert(native_ui_cargo(context, &view, 0) && native_ui_animation_pending(context));
  native_ui_advance(context, 120); /* A second context forbids global clock advance. */
  assert(native_ui_animation_pending(context));
  lv_mem_monitor_t memory;
  lv_mem_monitor(&memory);
  printf("LVGL pool with two contexts: %zu used / %zu total bytes\n",
         memory.total_size - memory.free_size, memory.total_size);
  native_ui_destroy(second);
  view.focus = 0;
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
static void copy_snapshot_file(const char *source, const char *destination, int optional) {
  FILE *input = fopen(source, "rb");
  if (!input && optional && errno == ENOENT) return;
  assert(input);
  FILE *output = fopen(destination, "wb");
  assert(output);
  uint8_t bytes[4096];
  size_t count;
  while ((count = fread(bytes, 1, sizeof(bytes), input)) != 0)
    assert(fwrite(bytes, 1, count, output) == count);
  assert(!ferror(input));
  assert(!fclose(input) && !fclose(output));
}
static void actual_save_motion(const char *directory, const char *source) {
  /* Normal loaders can migrate/journal. Give them a disposable snapshot, never
   * the source world's files. Rendering itself must preserve the loaded game. */
  char temporary[470];
  int length = snprintf(temporary, sizeof(temporary), "%s/cargo-ui-save-XXXXXX", directory);
  assert(length > 0 && (size_t)length < sizeof(temporary) && mkdtemp(temporary));
  char snapshot[512];
  length = snprintf(snapshot, sizeof(snapshot), "%s/world.save", temporary);
  assert(length > 0 && (size_t)length < sizeof(snapshot));
  const char *suffixes[] = {"", ".kit", ".kit.required"};
  for (unsigned index = 0; index < 3; ++index) {
    char input[560], output[560];
    length = snprintf(input, sizeof(input), "%s%s", source, suffixes[index]);
    assert(length > 0 && (size_t)length < sizeof(input));
    length = snprintf(output, sizeof(output), "%s%s", snapshot, suffixes[index]);
    assert(length > 0 && (size_t)length < sizeof(output));
    copy_snapshot_file(input, output, index != 0);
  }
  SelectedLab lab;
  selected_lab_init(&lab);
  assert(selected_lab_load(&lab, snapshot, 100));
  DeviceKit kit;
  assert(kit_init(&kit, &lab, 100));
  kit.companion.page = kit.companion.mode = COMP_CARGO;
  CompanionCargoView view;
  assert(kit_cargo_projection(&kit, &view));
  GameState before = lab.game;
  rendering_and_motion(directory, &view);
  assert(!memcmp(&before, &lab.game, sizeof(before)));
  char metadata[560];
  length = snprintf(metadata, sizeof(metadata), "%s/cargo-projection.txt", directory);
  assert(length > 0 && (size_t)length < sizeof(metadata));
  FILE *output = fopen(metadata, "w");
  assert(output);
  fprintf(output, "Copied actual Cargo projection: %s\nSupplies: %u/%u/%u\nCapsules: %u\n"
          "Controlled presentation focus0->1, still and0/60/120ms. No live input timing.\n",
          view.identity, view.supplies[0], view.supplies[1], view.supplies[2], view.capsules);
  assert(!fclose(output));
  const char *cleanup[] = {"", ".kit", ".kit.required", ".lock", ".kit.lock", ".kit.required.lock"};
  for (unsigned index = 0; index < sizeof(cleanup)/sizeof(cleanup[0]); ++index) {
    char path[560];
    length = snprintf(path, sizeof(path), "%s%s", snapshot, cleanup[index]);
    assert(length > 0 && (size_t)length < sizeof(path));
    if (unlink(path)) assert(errno == ENOENT);
  }
  assert(!rmdir(temporary));
}
int main(int argc, char **argv) {
  image_fidelity();
  projection_truth();
  if (argc == 3) actual_save_motion(argv[1], argv[2]);
  else rendering_and_motion(argc == 2 ? argv[1] : NULL, NULL);
  puts("Companion Cargo UI checks passed");
  return 0;
}
