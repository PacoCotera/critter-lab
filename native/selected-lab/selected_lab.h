#ifndef SELECTED_LAB_H
#define SELECTED_LAB_H
#include "game_rules.h"
#include "pip_genetics.h"
#include <stdint.h>
#include <stdio.h>
#define SELECTED_LAB_WIDTH 1024u
#define SELECTED_LAB_HEIGHT 600u
typedef enum {
  V1_HOME,
  V1_EXPEDITION,
  V1_CARGO,
  V1_SAMPLES,
  V1_STUDIES,
  V1_FINDING,
  V1_CREATE,
  V1_INCUBATION,
  V1_REVEAL,
  V1_HABITAT,
  V1_STUDY_REVIEW,
  V1_DISCARD_REVIEW,
  V1_CRITTERS,
  V1_LIBRARY,
  V1_LIBRARY_FINDING
} SelectedPage;
typedef enum {
  SELECTED_UP_DOWN,
  SELECTED_UP_UP,
  SELECTED_DOWN_DOWN,
  SELECTED_DOWN_UP,
  SELECTED_LEFT_DOWN,
  SELECTED_LEFT_UP,
  SELECTED_RIGHT_DOWN,
  SELECTED_RIGHT_UP,
  SELECTED_RESEARCH_DOWN,
  SELECTED_RESEARCH_UP,
  SELECTED_CRITTERS_DOWN,
  SELECTED_CRITTERS_UP,
  SELECTED_LIBRARY_DOWN,
  SELECTED_LIBRARY_UP,
  SELECTED_HABITAT_DOWN,
  SELECTED_HABITAT_UP,
  SELECTED_CONFIRM_DOWN,
  SELECTED_CONFIRM_UP,
  SELECTED_BACK_DOWN,
  SELECTED_BACK_UP,
  SELECTED_CANCEL,
  SELECTED_SUSPEND,
  SELECTED_RESUME,
  SELECTED_READY
} SelectedInput;
typedef struct {
  int held, allowed;
  unsigned revision, interaction_epoch;
} SelectedGesture;
typedef struct {
  SelectedPage page;
  unsigned focus, sample, study, resident, discard_resource, revision,
      page_revision, interaction_epoch, minimum_action_revision,
      acknowledged_revision, acknowledged_interaction_epoch;
  int ready, suspended, storage_error, kit_mode;
  SelectedGesture gestures[10];
  unsigned workspace, library_index;
  SelectedPage workspace_page[4];
  unsigned workspace_focus[4], workspace_sample[4], workspace_study[4],
      workspace_resident[4];
  GameState game;
  char save_path[512];
  char message[96];
  uint32_t clock;
} SelectedLab;
typedef struct {
  SelectedPage page;
  unsigned focus, sample, study, resident, discard_resource, workspace,
      library_index;
  SelectedPage workspace_page[4];
  unsigned workspace_focus[4], workspace_sample[4], workspace_study[4],
      workspace_resident[4];
  char message[96];
} SelectedLabContext;

void selected_lab_capture_context(const SelectedLab *lab,
                                  SelectedLabContext *context);
void selected_lab_open_reception(SelectedLab *lab);
void selected_lab_restore_context(SelectedLab *lab,
                                  const SelectedLabContext *context);
void selected_lab_init(SelectedLab *lab);
int selected_lab_load(SelectedLab *lab, const char *path, uint32_t clock);
void selected_lab_tick(SelectedLab *lab, uint32_t clock);
void selected_lab_tick_devices(SelectedLab *lab, uint32_t clock, int expedition,
                               int incubation);
void selected_lab_input(SelectedLab *lab, SelectedInput input, int delta,
                        unsigned frame);
const char *selected_lab_page(const SelectedLab *lab);
const char *selected_lab_focus(const SelectedLab *lab);
unsigned selected_lab_options(const SelectedLab *lab);
const char *selected_lab_option(const SelectedLab *lab, unsigned option);
int selected_lab_library_entry(const SelectedLab *lab, unsigned option,
                               unsigned *sample_result, unsigned *study_result);
void selected_lab_row(const SelectedLab *lab, unsigned row,
                      uint8_t pixels[SELECTED_LAB_WIDTH * 3]);
/* Shared existing asset treatment for the connected native device views. */
void selected_lab_sprite_row(unsigned asset, int x, int y, unsigned width,
                             unsigned height, unsigned row,
                             uint8_t pixels[SELECTED_LAB_WIDTH * 3]);
int selected_lab_bmp(const SelectedLab *lab, FILE *output);
#endif
