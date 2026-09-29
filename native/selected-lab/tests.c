#include "selected_lab.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static void ready(SelectedLab *lab) {
  selected_lab_input(lab, SELECTED_READY, 0, lab->revision);
}
static void press(SelectedLab *lab) {
  selected_lab_input(lab, SELECTED_CONFIRM_DOWN, 0, lab->revision);
  selected_lab_input(lab, SELECTED_CONFIRM_UP, 0, lab->revision);
}
static void frame(SelectedLab *lab) {
  FILE *output = tmpfile();
  assert(output);
  assert(selected_lab_bmp(lab, output));
  assert(ftell(output) == 54L + SELECTED_LAB_WIDTH * SELECTED_LAB_HEIGHT * 3L);
  assert(fclose(output) == 0);
}
int main(void) {
  SelectedLab lab;
  selected_lab_init(&lab);
  press(&lab);
  assert(lab.page == V1_HOME);
  ready(&lab);
  unsigned old = lab.revision;
  selected_lab_input(&lab, SELECTED_CONFIRM_DOWN, 0, old);
  selected_lab_input(&lab, SELECTED_DOWN_DOWN, 0, old);
  selected_lab_input(&lab, SELECTED_DOWN_UP, 0, old);
  selected_lab_input(&lab, SELECTED_CONFIRM_UP, 0, old);
  assert(lab.page == V1_HOME && lab.focus == 0);
  ready(&lab);
  selected_lab_input(&lab, SELECTED_DOWN_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_DOWN_UP, 0, lab.revision);
  assert(lab.focus == 1 && !lab.ready);
  /* A status tick queued ahead of a press changes pixels but not this action. */
  SelectedLab timed;
  selected_lab_init(&timed);
  char timed_path[128];
  snprintf(timed_path, sizeof(timed_path), "/tmp/beecho-timed-%ld.save",
           (long)getpid());
  strcpy(timed.save_path, timed_path);
  timed.game.expedition_active = 1;
  strcpy(timed.game.expedition_id, "timed-survey");
  timed.game.expedition_data = 100;
  timed.clock = 10;
  game_rules_resume_runtime(&timed.game, timed.clock);
  assert(game_state_save(timed_path, &timed.game) == 0);
  ready(&timed);
  unsigned displayed = timed.revision;
  unsigned action_epoch = timed.interaction_epoch;
  selected_lab_tick(&timed, 11);
  assert(timed.revision > displayed && !timed.ready);
  assert(timed.interaction_epoch == action_epoch);
  selected_lab_input(&timed, SELECTED_DOWN_DOWN, 0, displayed);
  selected_lab_input(&timed, SELECTED_DOWN_UP, 0, displayed);
  assert(timed.focus == 1);
  /* The first cargo and survey completion change available actions. */
  selected_lab_init(&timed);
  strcpy(timed.save_path, timed_path);
  timed.game.expedition_active = 1;
  strcpy(timed.game.expedition_id, "first-cargo");
  timed.clock = 10;
  game_rules_resume_runtime(&timed.game, timed.clock);
  assert(game_state_save(timed_path, &timed.game) == 0);
  ready(&timed);
  displayed = timed.revision;
  selected_lab_tick(&timed, 11);
  assert(timed.interaction_epoch != timed.acknowledged_interaction_epoch);
  selected_lab_input(&timed, SELECTED_CONFIRM_DOWN, 0, displayed);
  selected_lab_input(&timed, SELECTED_CONFIRM_UP, 0, displayed);
  assert(timed.page == V1_HOME);
  selected_lab_init(&timed);
  strcpy(timed.save_path, timed_path);
  timed.game.expedition_active = 1;
  strcpy(timed.game.expedition_id, "completion");
  timed.game.expedition_data = 100;
  timed.game.expedition_elapsed = 59;
  timed.clock = 10;
  game_rules_resume_runtime(&timed.game, timed.clock);
  assert(game_state_save(timed_path, &timed.game) == 0);
  ready(&timed);
  displayed = timed.revision;
  selected_lab_tick(&timed, 11);
  assert(timed.interaction_epoch != timed.acknowledged_interaction_epoch);
  unlink(timed_path);
  press(&lab);
  assert(lab.page == V1_HOME);
  ready(&lab);
  press(&lab);
  assert(lab.page == V1_EXPEDITION);
  ready(&lab);
  selected_lab_input(&lab, SELECTED_BACK_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_BACK_UP, 0, lab.revision);
  assert(lab.page == V1_HOME);
  selected_lab_input(&lab, SELECTED_CONFIRM_DOWN, 0, old);
  selected_lab_input(&lab, SELECTED_CONFIRM_UP, 0, lab.revision);
  assert(lab.page == V1_HOME);
  ready(&lab);
  selected_lab_input(&lab, SELECTED_CONFIRM_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_SUSPEND, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_RESUME, 0, lab.revision);
  ready(&lab);
  selected_lab_input(&lab, SELECTED_CONFIRM_UP, 0, lab.revision);
  assert(lab.page == V1_HOME);
  /* Workspace keys expose only retained player knowledge and never commit. */
  lab.game.sample_count = 1;
  strcpy(lab.game.samples[0].id, "sample-test");
  lab.game.samples[0].decoded_studies = 1;
  ready(&lab);
  selected_lab_input(&lab, SELECTED_LIBRARY_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_LIBRARY_UP, 0, lab.revision);
  assert(lab.page == V1_LIBRARY && selected_lab_options(&lab) == 1);
  ready(&lab);
  selected_lab_input(&lab, SELECTED_RIGHT_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_RIGHT_UP, 0, lab.revision);
  assert(lab.page == V1_LIBRARY_FINDING && lab.study == 0);
  ready(&lab);
  selected_lab_input(&lab, SELECTED_CRITTERS_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_CRITTERS_UP, 0, lab.revision);
  assert(lab.page == V1_CRITTERS && lab.game.last_operation_sequence == 0);
  ready(&lab);
  selected_lab_input(&lab, SELECTED_LIBRARY_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_LIBRARY_UP, 0, lab.revision);
  assert(lab.page == V1_LIBRARY_FINDING);
  /* Right never starts a study, expedition, incubation or care operation. */
  lab.page = V1_STUDY_REVIEW;
  ready(&lab);
  selected_lab_input(&lab, SELECTED_RIGHT_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_RIGHT_UP, 0, lab.revision);
  assert(lab.page == V1_STUDY_REVIEW && lab.game.last_operation_sequence == 0);
  /* Home focus previews cannot mutate the world; only non-Home Confirm enters.
   */
  SelectedLab navigation;
  selected_lab_init(&navigation);
  ready(&navigation);
  press(&navigation);
  assert(navigation.page == V1_HOME && navigation.focus == 0);
  for (unsigned home_focus = 1; home_focus <= 4; ++home_focus) {
    ready(&navigation);
    selected_lab_input(&navigation, SELECTED_DOWN_DOWN, 0, navigation.revision);
    selected_lab_input(&navigation, SELECTED_DOWN_UP, 0, navigation.revision);
    assert(navigation.page == V1_HOME && navigation.focus == home_focus);
    assert(navigation.game.last_operation_sequence == 0);
    ready(&navigation);
    selected_lab_input(&navigation, SELECTED_CONFIRM_DOWN, 0,
                       navigation.revision);
    selected_lab_input(&navigation, SELECTED_CONFIRM_UP, 0,
                       navigation.revision);
    assert(navigation.page != V1_HOME);
    assert(navigation.game.last_operation_sequence == 0);
    ready(&navigation);
    selected_lab_input(&navigation, SELECTED_BACK_DOWN, 0, navigation.revision);
    selected_lab_input(&navigation, SELECTED_BACK_UP, 0, navigation.revision);
    assert(navigation.page == V1_HOME && navigation.focus == home_focus);
  }
  char path[128];
  snprintf(path, sizeof(path), "/tmp/beecho-ui-recovery-%ld.save",
           (long)getpid());
  GameState durable;
  game_state_init(&durable);
  durable.data = 500;
  assert(game_state_save(path, &durable) == 0);
  selected_lab_init(&lab);
  assert(selected_lab_load(&lab, path, 10));
  lab.storage_error = 1;
  lab.game.data = 0;
  selected_lab_input(&lab, SELECTED_RESUME, 0, lab.revision);
  assert(!lab.storage_error && lab.game.data == 500 && !lab.ready);
  unlink(path);
  char lockpath[140];
  snprintf(lockpath, sizeof(lockpath), "%s.lock", path);
  unlink(lockpath);
  uint8_t pixels[SELECTED_LAB_WIDTH * 3];
  for (unsigned page = V1_HOME; page <= V1_LIBRARY_FINDING; page++) {
    lab.page = (SelectedPage)page;
    for (unsigned row = 0; row < SELECTED_LAB_HEIGHT; row++)
      selected_lab_row(&lab, row, pixels);
  }
  /* Populated rows exercise fonts and assets absent from empty-page fixtures.
   */
  lab.game.sample_count = 2;
  strcpy(lab.game.samples[0].id, "sample-0001");
  strcpy(lab.game.samples[1].id, "sample-0002");
  lab.game.samples[0].decoded_studies = 31;
  lab.game.samples[1].decoded_studies = 1;
  lab.game.individual_count = 3;
  strcpy(lab.game.individuals[0].id, "resident-hidden");
  strcpy(lab.game.individuals[1].id, "resident-0001");
  strcpy(lab.game.individuals[2].id, "resident-0002");
  lab.game.individuals[1].revealed = lab.game.individuals[2].revealed = 1;
  lab.page = V1_HOME;
  for (lab.focus = 0; lab.focus < 5; ++lab.focus)
    frame(&lab);
  lab.page = V1_LIBRARY;
  for (lab.focus = 0; lab.focus < selected_lab_options(&lab); ++lab.focus)
    frame(&lab);
  lab.page = V1_LIBRARY_FINDING;
  lab.sample = lab.focus = 0;
  for (lab.study = 0; lab.study < 5; ++lab.study)
    frame(&lab);
  lab.page = V1_STUDIES;
  lab.focus = 5;
  frame(&lab);
  lab.page = V1_HOME;
  lab.workspace = 4;
  lab.workspace_focus[1] = 1;
  lab.workspace_resident[1] = 0;
  ready(&lab);
  selected_lab_input(&lab, SELECTED_CRITTERS_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_CRITTERS_UP, 0, lab.revision);
  assert(lab.page == V1_CRITTERS && lab.focus == 1 && lab.resident == 2);
  frame(&lab);
  ready(&lab);
  selected_lab_input(&lab, SELECTED_HABITAT_DOWN, 0, lab.revision);
  selected_lab_input(&lab, SELECTED_HABITAT_UP, 0, lab.revision);
  assert(lab.page == V1_HABITAT && lab.resident == 2);
  frame(&lab);
  /* Maximum valid stock must not paint over the header's right-hand inset. */
  SelectedLab empty_header, full_header;
  selected_lab_init(&empty_header);
  full_header = empty_header;
  full_header.game.data = full_header.game.energy = full_header.game.essence =
      1000000;
  uint8_t empty_row[SELECTED_LAB_WIDTH * 3], full_row[SELECTED_LAB_WIDTH * 3];
  for (unsigned row = 60; row < 96; ++row) {
    selected_lab_row(&empty_header, row, empty_row);
    selected_lab_row(&full_header, row, full_row);
    assert(memcmp(empty_row + 972 * 3, full_row + 972 * 3,
                  (SELECTED_LAB_WIDTH - 972) * 3) == 0);
  }
  puts("Native V1 input and frame checks passed");
  return 0;
}
