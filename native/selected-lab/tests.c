#include "selected_lab.h"
#include "assets.h"
#include <assert.h>
#include <string.h>

#ifdef NDEBUG
#error "Native checks must retain assertions, including Release builds"
#endif

static void ready(SelectedLab *lab) { selected_lab_input(lab, SELECTED_READY, 0, lab->revision); }
static void press(SelectedLab *lab) {
  selected_lab_input(lab, SELECTED_CONFIRM_DOWN, 0, lab->revision);
  selected_lab_input(lab, SELECTED_CONFIRM_UP, 0, lab->revision);
}

int main(void) {
  SelectedLab lab;
  selected_lab_init(&lab);
  selected_lab_input(&lab, SELECTED_CONFIRM_DOWN, 0, lab.revision);
  ready(&lab);
  selected_lab_input(&lab, SELECTED_CONFIRM_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_CONFIRM_UP, 0, lab.revision);
  assert(lab.page == SELECTED_STUDY); /* Blocked hold stays consumed. */
  press(&lab);
  assert(lab.page == SELECTED_START_PREVIEW && lab.focus == SELECTED_RETURN);
  selected_lab_input(&lab, SELECTED_BACK_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_BACK_UP, 0, lab.revision);
  assert(lab.page == SELECTED_STUDY && lab.focus == SELECTED_START);

  ready(&lab);
  unsigned old_frame = lab.revision;
  selected_lab_input(&lab, SELECTED_CONFIRM_DOWN, 0, old_frame);
  selected_lab_input(&lab, SELECTED_ROTATE, 1, old_frame);
  selected_lab_input(&lab, SELECTED_READY, 0, old_frame);
  assert(!lab.ready && lab.focus == SELECTED_CROWN_TARGET);
  ready(&lab);
  selected_lab_input(&lab, SELECTED_CONFIRM_UP, 0, lab.revision);
  assert(lab.page == SELECTED_STUDY); /* Redraw invalidates held activation. */
  press(&lab);
  assert(lab.page == SELECTED_CROWN);
  selected_lab_input(&lab, SELECTED_BACK_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_BACK_UP, 0, lab.revision);
  assert(lab.focus == SELECTED_CROWN_TARGET);

  ready(&lab);
  selected_lab_input(&lab, SELECTED_CONFIRM_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_CANCEL, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_CONFIRM_UP, 0, lab.revision);
  assert(lab.page == SELECTED_STUDY);
  selected_lab_input(&lab, SELECTED_CONFIRM_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_SUSPEND, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_RESUME, 0, lab.revision);
  ready(&lab);
  selected_lab_input(&lab, SELECTED_CONFIRM_UP, 0, lab.revision);
  assert(lab.page == SELECTED_STUDY);
  selected_lab_input(&lab, SELECTED_CONFIRM_DOWN, 0, lab.revision - 1);
  selected_lab_input(&lab, SELECTED_CONFIRM_UP, 0, lab.revision);
  assert(lab.page == SELECTED_STUDY); /* Stale client cannot arm Confirm. */

  selected_lab_init(&lab);
  old_frame = lab.revision;
  ready(&lab);
  press(&lab);
  assert(lab.page == SELECTED_START_PREVIEW);
  unsigned preview_frame = lab.revision;
  selected_lab_input(&lab, SELECTED_BACK_DOWN, 0, preview_frame);
  selected_lab_input(&lab, SELECTED_BACK_UP, 0, preview_frame);
  unsigned returned_frame = lab.revision;
  selected_lab_input(&lab, SELECTED_ROTATE, 1, old_frame);
  selected_lab_input(&lab, SELECTED_ROTATE, 1, preview_frame);
  selected_lab_input(&lab, SELECTED_ROTATE, 1, returned_frame + 1);
  assert(lab.focus == SELECTED_START && lab.revision == returned_frame);
  selected_lab_input(&lab, SELECTED_ROTATE, 1, returned_frame);
  selected_lab_input(&lab, SELECTED_ROTATE, 1, returned_frame);
  assert(lab.focus == SELECTED_EYE_TARGET && !lab.ready);
  selected_lab_input(&lab, SELECTED_READY, 0, returned_frame + 1);
  assert(!lab.ready); /* One latest pending target; no queued intermediate frame. */
  ready(&lab);
  assert(lab.ready && lab.focus == SELECTED_EYE_TARGET);
  unsigned before_suspend = lab.revision;
  selected_lab_input(&lab, SELECTED_SUSPEND, 0, before_suspend);
  selected_lab_input(&lab, SELECTED_RESUME, 0, before_suspend);
  unsigned resumed_frame = lab.revision;
  selected_lab_input(&lab, SELECTED_ROTATE, -1, before_suspend);
  assert(lab.focus == SELECTED_EYE_TARGET && lab.revision == resumed_frame);
  selected_lab_input(&lab, SELECTED_ROTATE, -1, resumed_frame);
  assert(lab.focus == SELECTED_CROWN_TARGET && !lab.ready);
  selected_lab_input(&lab, SELECTED_READY, 0, resumed_frame);
  assert(!lab.ready);
  ready(&lab);
  assert(lab.ready);

  selected_lab_init(&lab);
  uint8_t pixels[SELECTED_LAB_WIDTH * 3];
  selected_lab_row(&lab, 48, pixels);
  assert(!memcmp(pixels + 470 * 3, selected_sprites[SPRITE_DATA].pixels, 3));
  FILE *output = tmpfile();
  assert(output && selected_lab_bmp(&lab, output));
  assert(ftell(output) == (long)(54u + SELECTED_LAB_WIDTH * SELECTED_LAB_HEIGHT * 3u));
  fclose(output);
  puts("Native input/readiness and asset/BMP boundary checks passed.");
  return 0;
}
