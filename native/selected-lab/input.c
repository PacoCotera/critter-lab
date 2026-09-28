#include "selected_lab.h"
#include <string.h>

static void changed(SelectedLab *lab) {
  ++lab->revision;
  lab->ready = 0;
}

static void entered_page(SelectedLab *lab) {
  changed(lab);
  /* A return or lifecycle boundary invalidates navigation from the prior view. */
  lab->page_revision = lab->revision;
}

void selected_lab_init(SelectedLab *lab) {
  memset(lab, 0, sizeof(*lab));
  lab->page = SELECTED_STUDY;
  lab->focus = lab->return_focus = SELECTED_START;
  lab->revision = 1;
  lab->page_revision = 1;
}

const char *selected_lab_page(const SelectedLab *lab) {
  static const char *const pages[] = {"study", "start-preview", "crown", "eye-ring"};
  return pages[lab->page];
}

const char *selected_lab_focus(const SelectedLab *lab) {
  static const char *const targets[] = {"start", "crown", "eye-ring", "return"};
  return targets[lab->focus];
}

void selected_lab_input(SelectedLab *lab, SelectedInput input, int delta, unsigned frame) {
  if (input == SELECTED_READY) {
    if (frame == lab->revision && !lab->suspended) lab->ready = 1;
    return;
  }
  if (input == SELECTED_CANCEL || input == SELECTED_SUSPEND || input == SELECTED_RESUME) {
    memset(&lab->confirm, 0, sizeof(lab->confirm));
    memset(&lab->back, 0, sizeof(lab->back));
    if (input != SELECTED_CANCEL) {
      lab->suspended = input == SELECTED_SUSPEND;
      entered_page(lab);
    }
    return;
  }
  if (input == SELECTED_ROTATE) {
    if (!lab->suspended && lab->page == SELECTED_STUDY && delta && frame >= lab->page_revision && frame <= lab->revision) {
      lab->focus = (SelectedFocus)(((int)lab->focus + (delta > 0 ? 1 : 2)) % 3);
      changed(lab);
    }
    return;
  }
  int back = input == SELECTED_BACK_DOWN || input == SELECTED_BACK_UP;
  SelectedGesture *gesture = back ? &lab->back : &lab->confirm;
  if (input == SELECTED_CONFIRM_DOWN || input == SELECTED_BACK_DOWN) {
    if (!gesture->held) {
      gesture->held = 1;
      gesture->revision = lab->revision;
      gesture->allowed = !lab->suspended && (back || (lab->ready && frame == lab->revision));
    }
    return;
  }
  if (input != SELECTED_CONFIRM_UP && input != SELECTED_BACK_UP) return;
  if (!gesture->held) return;
  int allowed = gesture->allowed;
  unsigned pressed_frame = gesture->revision;
  memset(gesture, 0, sizeof(*gesture));
  if (!allowed || lab->suspended) return;
  if (back) {
    if (lab->page != SELECTED_STUDY) {
      lab->page = SELECTED_STUDY;
      lab->focus = lab->return_focus;
      entered_page(lab);
    }
    return;
  }
  if (!lab->ready || pressed_frame != lab->revision || frame != lab->revision) return;
  if (lab->page != SELECTED_STUDY) {
    lab->page = SELECTED_STUDY;
    lab->focus = lab->return_focus;
  } else {
    lab->return_focus = lab->focus;
    lab->page = lab->focus == SELECTED_START ? SELECTED_START_PREVIEW : lab->focus == SELECTED_CROWN_TARGET ? SELECTED_CROWN : SELECTED_EYE_RING;
    lab->focus = SELECTED_RETURN;
  }
  entered_page(lab);
}
