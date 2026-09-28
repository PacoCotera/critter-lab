#ifndef SELECTED_LAB_H
#define SELECTED_LAB_H
#include <stdint.h>
#include <stdio.h>

#define SELECTED_LAB_WIDTH 1024u
#define SELECTED_LAB_HEIGHT 600u

typedef enum { SELECTED_STUDY, SELECTED_START_PREVIEW, SELECTED_CROWN, SELECTED_EYE_RING } SelectedPage;
typedef enum { SELECTED_START, SELECTED_CROWN_TARGET, SELECTED_EYE_TARGET, SELECTED_RETURN } SelectedFocus;
typedef enum {
  SELECTED_ROTATE, SELECTED_CONFIRM_DOWN, SELECTED_CONFIRM_UP,
  SELECTED_BACK_DOWN, SELECTED_BACK_UP, SELECTED_CANCEL,
  SELECTED_SUSPEND, SELECTED_RESUME, SELECTED_READY
} SelectedInput;

typedef struct { int held, allowed; unsigned revision; } SelectedGesture;
typedef struct {
  SelectedPage page;
  SelectedFocus focus, return_focus;
  unsigned revision;
  int ready, suspended;
  SelectedGesture confirm, back;
} SelectedLab;

void selected_lab_init(SelectedLab *lab);
void selected_lab_input(SelectedLab *lab, SelectedInput input, int delta, unsigned frame);
const char *selected_lab_page(const SelectedLab *lab);
const char *selected_lab_focus(const SelectedLab *lab);
void selected_lab_row(const SelectedLab *lab, unsigned row, uint8_t pixels[SELECTED_LAB_WIDTH * 3]);
int selected_lab_bmp(const SelectedLab *lab, FILE *output);
#endif
