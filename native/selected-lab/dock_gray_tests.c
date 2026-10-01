#define _POSIX_C_SOURCE 200809L
#include "kit.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

/* Exercise the actual native frame boundary: every channel must be one of
 * the panel's four levels, with both middle levels used for hierarchy. */
static void check_frame(const DeviceKit *kit) {
  FILE *frame = tmpfile();
  assert(frame && kit_bmp(kit, KIT_DOCK, frame));
  assert(fseek(frame, 54, SEEK_SET) == 0);
  unsigned levels[4] = {0};
  for (unsigned pixel = 0; pixel < 792u * 272u; ++pixel) {
    int blue = fgetc(frame), green = fgetc(frame), red = fgetc(frame);
    assert(blue >= 0 && blue == green && green == red);
    assert(blue == 0 || blue == 85 || blue == 170 || blue == 255);
    ++levels[(unsigned)blue / 85u];
  }
  assert(levels[0] && levels[1] && levels[2] && levels[3]);
  assert(fclose(frame) == 0);
}

int main(void) {
  char directory[] = "/tmp/critter-dock-gray-XXXXXX";
  assert(mkdtemp(directory));
  char path[512];
  snprintf(path, sizeof(path), "%s/world", directory);
  SelectedLab lab;
  selected_lab_init(&lab);
  assert(selected_lab_load(&lab, path, 100));
  DeviceKit kit;
  assert(kit_init(&kit, &lab, 100));
  uint64_t sequence = lab.game.last_operation_sequence;
  for (unsigned focus = 0; focus < 3; ++focus) {
    kit.dock.focus = focus;
    check_frame(&kit);
  }
  assert(kit_link(&kit, KIT_DOCK, 0));
  check_frame(&kit);
  kit.dock.page = 2;
  kit.dock.focus = 0;
  check_frame(&kit);
  assert(lab.game.last_operation_sequence == sequence);
  char sidecar[580];
  snprintf(sidecar, sizeof(sidecar), "%s.kit", path);
  unlink(sidecar);
  snprintf(sidecar, sizeof(sidecar), "%s.kit.required", path);
  unlink(sidecar);
  unlink(path);
  rmdir(directory);
  puts("Dock native four-level raster checks passed");
  return 0;
}
