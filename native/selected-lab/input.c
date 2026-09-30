#include "selected_lab.h"
#include <stdio.h>
#include <string.h>
static void changed(SelectedLab *lab) {
  ++lab->revision;
  lab->ready = 0;
}
static void interaction_changed(SelectedLab *lab) {
  ++lab->interaction_epoch;
  changed(lab);
  lab->minimum_action_revision = lab->revision;
}
static unsigned page_workspace(SelectedPage page) {
  switch (page) {
  case V1_SAMPLES:
  case V1_STUDIES:
  case V1_FINDING:
  case V1_CREATE:
  case V1_CREATE_REVIEW:
  case V1_STUDY_REVIEW:
    return 0;
  case V1_CRITTERS:
    return 3;
  case V1_LIBRARY:
  case V1_LIBRARY_FINDING:
    return 2;
  case V1_HABITAT:
    return 3;
  default:
    return 4;
  }
}
static void remember_workspace(SelectedLab *lab) {
  if (lab->workspace >= 4)
    return;
  unsigned workspace = lab->workspace;
  lab->workspace_page[workspace] = lab->page;
  lab->workspace_focus[workspace] = lab->focus;
  lab->workspace_sample[workspace] = lab->sample;
  lab->workspace_study[workspace] = lab->study;
  lab->workspace_resident[workspace] = lab->resident;
}
static void enter(SelectedLab *lab, SelectedPage page) {
  remember_workspace(lab);
  if (!strncmp(lab->message, "Haul saved.", 11))
    lab->message[0] = '\0';
  lab->page = page;
  lab->workspace = page_workspace(page);
  lab->focus = 0;
  interaction_changed(lab);
  lab->page_revision = lab->revision;
}
/* Only navigation is captured. World state, clocks and gesture readiness stay
 * authoritative when a received haul temporarily replaces the current view. */
void selected_lab_capture_context(const SelectedLab *lab,
                                  SelectedLabContext *context) {
  context->page = lab->page;
  context->focus = lab->focus;
  context->sample = lab->sample;
  context->study = lab->study;
  context->resident = lab->resident;
  context->discard_resource = lab->discard_resource;
  context->workspace = lab->workspace;
  context->library_index = lab->library_index;
  context->creation_preference = lab->creation_preference;
  memcpy(context->workspace_page, lab->workspace_page,
         sizeof(lab->workspace_page));
  memcpy(context->workspace_focus, lab->workspace_focus,
         sizeof(lab->workspace_focus));
  memcpy(context->workspace_sample, lab->workspace_sample,
         sizeof(lab->workspace_sample));
  memcpy(context->workspace_study, lab->workspace_study,
         sizeof(lab->workspace_study));
  memcpy(context->workspace_resident, lab->workspace_resident,
         sizeof(lab->workspace_resident));
  memcpy(context->message, lab->message, sizeof(lab->message));
}
void selected_lab_open_reception(SelectedLab *lab) {
  enter(lab, V1_CARGO);
  lab->message[0] = 0;
  memset(lab->gestures, 0, sizeof(lab->gestures));
}
void selected_lab_restore_context(SelectedLab *lab,
                                  const SelectedLabContext *context) {
  lab->page = context->page;
  lab->focus = context->focus;
  lab->sample = context->sample;
  lab->study = context->study;
  lab->resident = context->resident;
  lab->discard_resource = context->discard_resource;
  lab->workspace = context->workspace;
  lab->library_index = context->library_index;
  lab->creation_preference = context->creation_preference;
  memcpy(lab->workspace_page, context->workspace_page,
         sizeof(lab->workspace_page));
  memcpy(lab->workspace_focus, context->workspace_focus,
         sizeof(lab->workspace_focus));
  memcpy(lab->workspace_sample, context->workspace_sample,
         sizeof(lab->workspace_sample));
  memcpy(lab->workspace_study, context->workspace_study,
         sizeof(lab->workspace_study));
  memcpy(lab->workspace_resident, context->workspace_resident,
         sizeof(lab->workspace_resident));
  memcpy(lab->message, context->message, sizeof(lab->message));
  memset(lab->gestures, 0, sizeof(lab->gestures));
  interaction_changed(lab);
  lab->page_revision = lab->revision;
}
static unsigned discovered_findings(const SelectedLab *lab) {
  unsigned count = 0;
  for (unsigned i = 0; i < lab->game.sample_count; ++i)
    for (unsigned study = 0; study < 5; ++study)
      count += (lab->game.samples[i].decoded_studies >> study) & 1u;
  return count;
}
int selected_lab_library_entry(const SelectedLab *lab, unsigned option,
                               unsigned *sample_result,
                               unsigned *study_result) {
  unsigned index = 0;
  for (unsigned sample = 0; sample < lab->game.sample_count; ++sample)
    for (unsigned study = 0; study < 5; ++study)
      if (lab->game.samples[sample].decoded_studies & (1u << study)) {
        if (index++ == option) {
          *sample_result = sample;
          *study_result = study;
          return 1;
        }
      }
  return 0;
}
static unsigned visible_residents(const SelectedLab *lab) {
  unsigned count = 0;
  for (unsigned i = 0; i < lab->game.individual_count; i++)
    count += lab->game.individuals[i].revealed;
  return count;
}
static void focus_resident(SelectedLab *lab) {
  unsigned index = 0;
  for (unsigned i = 0; i < lab->game.individual_count; ++i)
    if (lab->game.individuals[i].revealed && index++ == lab->focus) {
      lab->resident = i;
      return;
    }
}
void selected_lab_init(SelectedLab *lab) {
  memset(lab, 0, sizeof(*lab));
  game_state_init(&lab->game);
  lab->workspace_page[0] = V1_SAMPLES;
  lab->workspace_page[1] = V1_HOME;
  lab->workspace_page[2] = V1_LIBRARY;
  lab->workspace_page[3] = V1_HABITAT;
  lab->workspace = 4;
  lab->revision = lab->page_revision = lab->interaction_epoch = 1;
  lab->minimum_action_revision = 1;
}
int selected_lab_load(SelectedLab *lab, const char *path, uint32_t clock) {
  if (strlen(path) >= sizeof(lab->save_path))
    return 0;
  if (path != lab->save_path)
    strcpy(lab->save_path, path);
  lab->clock = clock;
  int loaded = game_state_load(path, &lab->game);
  if (loaded < 0) {
    lab->storage_error = 1;
    strcpy(lab->message, "Save unavailable. Existing data preserved.");
    return 0;
  }
  if (loaded > 0 && game_state_save(path, &lab->game) != 0) {
    lab->storage_error = 1;
    strcpy(lab->message, "Storage unavailable. Reload before continuing.");
    return 0;
  }
  lab->storage_error = 0;
  game_rules_resume_runtime(&lab->game, clock);
  return 1;
}
const char *selected_lab_page(const SelectedLab *lab) {
  static const char *names[] = {"home",     "expedition",   "cargo",
                                "samples",  "research",     "finding",
                                "creation", "incubation",   "reveal",
                                "habitat",  "study-review", "discard-review",
                                "residents", "library",      "library-finding",
                                "creation-review"};
  return names[lab->page];
}
unsigned selected_lab_options(const SelectedLab *lab) {
  switch (lab->page) {
  case V1_CRITTERS:
    return visible_residents(lab) ? visible_residents(lab) : 1;
  case V1_LIBRARY:
    return discovered_findings(lab) ? discovered_findings(lab) : 1;
  case V1_LIBRARY_FINDING:
    return 1;
  case V1_STUDY_REVIEW:
  case V1_DISCARD_REVIEW:
  case V1_CREATE_REVIEW:
    return 2;
  case V1_HOME:
    return 5;
  case V1_EXPEDITION:
    return lab->kit_mode ? 1 : lab->game.expedition_id[0] ? 2 : 3;
  case V1_CARGO:
    return 4;
  case V1_SAMPLES:
    return lab->game.sample_count ? lab->game.sample_count : 1;
  case V1_STUDIES:
    return 6;
  case V1_FINDING:
    return 1;
  case V1_CREATE:
    return 2;
  case V1_INCUBATION:
    return 1;
  case V1_REVEAL:
    return 1;
  case V1_HABITAT:
    return visible_residents(lab) ? 4 : 1;
  }
  return 1;
}
const char *selected_lab_option(const SelectedLab *lab, unsigned option) {
  static const char *home[] = {"Overview", "Explore", "Research", "Incubator",
                               "Habitat"};
  static const char *routes[] = {"Field survey", "Garden forage",
                                 "Weather watch"};
  static const char *cargo[] = {"Return + store haul", "Discard data pack",
                                "Discard energy pack", "Discard essence pack"};
  static const char *care[] = {"Spend time together", "Next resident",
                               "Residents", "Explore again"};
  switch (lab->page) {
  case V1_CRITTERS: {
    unsigned index = 0;
    for (unsigned i = 0; i < lab->game.individual_count; ++i)
      if (lab->game.individuals[i].revealed && index++ == option)
        return lab->game.individuals[i].id;
    return "Return to workbench";
  }
  case V1_LIBRARY: {
    unsigned index = 0;
    for (unsigned sample = 0; sample < lab->game.sample_count; ++sample)
      for (unsigned study = 0; study < 5; ++study)
        if (lab->game.samples[sample].decoded_studies & (1u << study)) {
          if (index++ == option) {
            static char label[96];
            snprintf(label, sizeof(label), "%s / %s",
                     lab->game.samples[sample].id, pip_study(study)->title);
            return label;
          }
        }
    return "No discoveries yet";
  }
  case V1_LIBRARY_FINDING:
    return "Back to library";
  case V1_STUDY_REVIEW:
    return option ? "Return to topics" : "Start research";
  case V1_DISCARD_REVIEW:
    return option ? "Keep this pack" : "Discard 1 pack";
  case V1_HOME:
    return home[option % 5];
  case V1_EXPEDITION:
    if (lab->game.expedition_id[0] &&
        !game_transfer_available(&lab->game))
      return option ? "Cargo" : "Finish expedition";
    return lab->game.expedition_id[0]
               ? (option                        ? "Cargo"
                  : lab->game.expedition_active ? "Keep exploring"
                  : "Review haul")
               : routes[option % 3];
  case V1_CARGO:
    return cargo[option % 4];
  case V1_SAMPLES:
    return lab->game.sample_count
               ? lab->game.samples[option % lab->game.sample_count].id
               : "Find a sample outdoors";
  case V1_STUDIES:
    return option < 5 ? pip_study(option)->title : "Prepare incubation";
  case V1_FINDING:
    return "Back to research";
  case V1_CREATE:
    return option ? "Pale markings" : "Plain coat / carries pale";
  case V1_CREATE_REVIEW:
    return option ? "Change supported form" : "Start incubation";
  case V1_INCUBATION:
    return lab->game.incubation_ready ? "Open incubation"
                                      : "Return to workbench";
  case V1_REVEAL:
    return "Meet in the habitat";
  case V1_HABITAT:
    return visible_residents(lab) ? care[option % 4]
                                  : "Explore for your first sample";
  }
  return "Return";
}
const char *selected_lab_focus(const SelectedLab *lab) {
  return selected_lab_option(lab, lab->focus);
}
static GameResult commit(SelectedLab *lab, GameCommand command) {
  if (lab->storage_error)
    return GAME_STORAGE;
  char id[64];
  snprintf(id, sizeof(id), "local-%llu",
           (unsigned long long)lab->game.last_operation_sequence + 1);
  command.sequence = lab->game.last_operation_sequence + 1;
  command.operation_id = id;
  GameResult result = lab->storage_error
                          ? GAME_STORAGE
                          : game_apply(lab->save_path, &lab->game, &command);
  if (result == GAME_COMMITTED_UNCERTAIN) {
    strcpy(lab->message,
           "Recorded; storage durability uncertain. Reload before continuing.");
    lab->storage_error = 1;
  } else if (result == GAME_OK || result == GAME_DUPLICATE) {
    if (command.type != GAME_COMMAND_EXPEDITION_TICK &&
        command.type != GAME_COMMAND_INCUBATION_TICK)
      strcpy(lab->message, "Saved");
  } else if (result == GAME_UNAVAILABLE)
    strcpy(lab->message, "More supplies or discoveries needed.");
  else if (result == GAME_STORAGE) {
    strcpy(lab->message, "Could not save. No change accepted.");
    lab->storage_error = 1;
  } else
    strcpy(lab->message, "Action unavailable. Your progress is safe.");
  interaction_changed(lab);
  return result;
}
static int selected_lab_held(const SelectedLab *lab) {
  for (unsigned i = 0; i < 10; ++i)
    if (lab->gestures[i].held)
      return 1;
  return 0;
}
void selected_lab_tick(SelectedLab *lab, uint32_t clock) {
  selected_lab_tick_devices(lab, clock, 1, 1);
}
void selected_lab_tick_devices(SelectedLab *lab, uint32_t clock, int expedition,
                               int incubation) {
  if (clock <= lab->clock)
    return;
  lab->clock = clock;
  if (lab->suspended || selected_lab_held(lab)) {
    game_rules_resume_runtime(&lab->game, clock);
    return;
  }
  GameCommand command = {0};
  command.data.monotonic_seconds = clock;
  if (expedition && lab->game.expedition_active) {
    unsigned before_cargo = lab->game.expedition_data +
                            lab->game.expedition_energy +
                            lab->game.expedition_essence;
    unsigned before_resources[] = {lab->game.expedition_data,
                                   lab->game.expedition_energy,
                                   lab->game.expedition_essence};
    int before_active = lab->game.expedition_active;
    unsigned before_epoch = lab->interaction_epoch;
    unsigned before_minimum = lab->minimum_action_revision;
    command.type = GAME_COMMAND_EXPEDITION_TICK;
    commit(lab, command);
    unsigned after_cargo = lab->game.expedition_data +
                           lab->game.expedition_energy +
                           lab->game.expedition_essence;
    int discard_eligibility_unchanged =
        (before_resources[0] >= GAME_PACK_SIZE) ==
            (lab->game.expedition_data >= GAME_PACK_SIZE) &&
        (before_resources[1] >= GAME_PACK_SIZE) ==
            (lab->game.expedition_energy >= GAME_PACK_SIZE) &&
        (before_resources[2] >= GAME_PACK_SIZE) ==
            (lab->game.expedition_essence >= GAME_PACK_SIZE);
    if (lab->game.expedition_active == before_active &&
        (before_cargo == 0) == (after_cargo == 0) &&
        discard_eligibility_unchanged && !lab->storage_error) {
      lab->interaction_epoch = before_epoch;
      lab->minimum_action_revision = before_minimum;
    }
  }
  if (incubation && lab->game.incubation_active &&
      !lab->game.incubation_ready) {
    unsigned before_epoch = lab->interaction_epoch;
    unsigned before_minimum = lab->minimum_action_revision;
    command.type = GAME_COMMAND_INCUBATION_TICK;
    commit(lab, command);
    if (!lab->game.incubation_ready && !lab->storage_error) {
      lab->interaction_epoch = before_epoch;
      lab->minimum_action_revision = before_minimum;
    }
  }
}
static void activate(SelectedLab *lab) {
  GameCommand command = {0};
  unsigned focus = lab->focus;
  switch (lab->page) {
  case V1_CRITTERS:
    if (!visible_residents(lab))
      enter(lab, V1_HABITAT);
    else {
      focus_resident(lab);
      enter(lab, V1_HABITAT);
    }
    return;
  case V1_LIBRARY:
    if (discovered_findings(lab)) {
      lab->library_index = focus;
      selected_lab_library_entry(lab, focus, &lab->sample, &lab->study);
      enter(lab, V1_LIBRARY_FINDING);
    }
    return;
  case V1_LIBRARY_FINDING:
    enter(lab, V1_LIBRARY);
    lab->focus = lab->library_index;
    return;
  case V1_HOME: {
    static const SelectedPage pages[] = {V1_HOME, V1_EXPEDITION, V1_SAMPLES,
                                         V1_INCUBATION, V1_HABITAT};
    if (!focus)
      break;
    enter(lab, pages[focus]);
    if (lab->page == V1_HABITAT && visible_residents(lab)) {
      for (unsigned i = 0; i < lab->game.individual_count; i++)
        if (lab->game.individuals[i].revealed) {
          lab->resident = i;
          break;
        }
    }
    break;
  }
  case V1_EXPEDITION:
    if (lab->kit_mode)
      return;
    if (lab->game.expedition_id[0]) {
      if (!focus && !game_transfer_available(&lab->game)) {
        command.type = GAME_COMMAND_EXPEDITION_FINISH;
        commit(lab, command);
      } else if (focus || !lab->game.expedition_active)
        enter(lab, V1_CARGO);
      else {
        strcpy(lab->message,
               "Gathering while you explore. Cargo keeps your haul.");
        interaction_changed(lab);
      }
    } else {
      command.type = GAME_COMMAND_EXPEDITION_START;
      command.data.expedition.kind = (GameExpeditionKind)focus;
      command.data.expedition.monotonic_seconds = lab->clock;
      commit(lab, command);
      lab->focus = 0;
      game_rules_resume_runtime(&lab->game, lab->clock);
    }
    break;
  case V1_CARGO:
    if (lab->kit_mode)
      return;
    if (!focus) {
      command.type = GAME_COMMAND_EXPEDITION_UNLOAD;
      if (commit(lab, command) == GAME_OK) {
        enter(lab, V1_SAMPLES);
        snprintf(lab->message, sizeof(lab->message),
                 "Haul saved. Lab stock: Data %u | Energy %u | Essence %u",
                 lab->game.data / GAME_SUPPLY_UNIT,
                 lab->game.energy / GAME_SUPPLY_UNIT,
                 lab->game.essence / GAME_SUPPLY_UNIT);
      }
    } else {
      lab->discard_resource = focus - 1;
      enter(lab, V1_DISCARD_REVIEW);
    }
    break;
  case V1_SAMPLES:
    if (lab->game.sample_count) {
      lab->sample = focus;
      enter(lab, V1_STUDIES);
    } else
      enter(lab, V1_EXPEDITION);
    break;
  case V1_STUDIES:
    if (focus == 5) {
      if (lab->game.samples[lab->sample].decoded_studies == 31)
        enter(lab, V1_CREATE);
      else {
        strcpy(lab->message, "Discover every region before incubation.");
        interaction_changed(lab);
      }
    } else {
      lab->study = focus;
      if (lab->game.samples[lab->sample].decoded_studies & (1u << focus))
        enter(lab, V1_FINDING);
      else
        enter(lab, V1_STUDY_REVIEW);
    }
    break;
  case V1_STUDY_REVIEW:
    if (focus) {
      enter(lab, V1_STUDIES);
      lab->focus = lab->study;
    } else {
      command.type = GAME_COMMAND_STUDY;
      command.data.study.sample = lab->sample;
      command.data.study.study = lab->study;
      if (commit(lab, command) == GAME_OK)
        enter(lab, V1_FINDING);
    }
    break;
  case V1_DISCARD_REVIEW:
    if (focus) {
      enter(lab, V1_CARGO);
      lab->focus = lab->discard_resource + 1;
    } else {
      command.type = GAME_COMMAND_EXPEDITION_DISCARD;
      command.data.discard.resource = (GameResource)lab->discard_resource;
      command.data.discard.quantity = GAME_PACK_SIZE;
      command.data.discard.confirm = 1;
      if (commit(lab, command) == GAME_OK) {
        enter(lab, V1_CARGO);
        lab->focus = lab->discard_resource + 1;
      }
    }
    break;
  case V1_FINDING:
    enter(lab, V1_STUDIES);
    lab->focus = lab->study;
    break;
  case V1_CREATE:
    lab->creation_preference = focus;
    enter(lab, V1_CREATE_REVIEW);
    break;
  case V1_CREATE_REVIEW:
    if (focus) {
      enter(lab, V1_CREATE);
      lab->focus = lab->creation_preference;
      break;
    }
    if (lab->game.samples[lab->sample].incubated) {
      strcpy(lab->message,
             "This sample already has a Beecho. Choose another sample.");
      interaction_changed(lab);
      break;
    }
    if (lab->game.individual_count >= GAME_MAX_INDIVIDUALS) {
      strcpy(lab->message, "Your 8 resident spaces are occupied. Explore or "
                           "visit your habitat.");
      interaction_changed(lab);
      break;
    }
    if (lab->game.incubation_active) {
      strcpy(lab->message, "An incubation is already active. Visit Incubator.");
      interaction_changed(lab);
      break;
    }
    command.type = GAME_COMMAND_INCUBATION_START;
    command.data.creation.sample = lab->sample;
    command.data.creation.preference = lab->creation_preference;
    command.data.creation.monotonic_seconds = lab->clock;
    if (commit(lab, command) == GAME_OK) {
      enter(lab, V1_INCUBATION);
      game_rules_resume_runtime(&lab->game, lab->clock);
    }
    break;
  case V1_INCUBATION:
    if (lab->game.incubation_ready) {
      command.type = GAME_COMMAND_INCUBATION_OPEN;
      if (commit(lab, command) == GAME_OK) {
        lab->resident = lab->game.individual_count - 1;
        enter(lab, V1_REVEAL);
      }
    } else
      enter(lab, V1_HOME);
    break;
  case V1_REVEAL:
    command.type = GAME_COMMAND_HABITAT_VISIT;
    command.data.habitat.individual = lab->resident;
    command.data.habitat.habitat = 1;
    if (commit(lab, command) >= GAME_OK)
      enter(lab, V1_HABITAT);
    break;
  case V1_HABITAT:
    if (!visible_residents(lab) || focus == 3)
      enter(lab, V1_EXPEDITION);
    else if (focus == 2) {
      enter(lab, V1_CRITTERS);
      unsigned index = 0;
      for (unsigned i = 0; i < lab->game.individual_count; ++i)
        if (lab->game.individuals[i].revealed) {
          if (i == lab->resident) {
            lab->focus = index;
            break;
          }
          ++index;
        }
    } else if (focus == 1) {
      do {
        lab->resident = (lab->resident + 1) % lab->game.individual_count;
      } while (!lab->game.individuals[lab->resident].revealed);
      interaction_changed(lab);
    } else {
      command.type = GAME_COMMAND_CARE_VISIT;
      command.data.individual = lab->resident;
      if (commit(lab, command) == GAME_OK)
        strcpy(lab->message, "Pip perks up and settles beside you.");
    }
    break;
  }
}
void selected_lab_input(SelectedLab *lab, SelectedInput input, int delta,
                        unsigned frame) {
  if (input == SELECTED_RESUME && lab->storage_error && lab->save_path[0]) {
    if (selected_lab_load(lab, lab->save_path, lab->clock)) {
      enter(lab, V1_HOME);
      strcpy(lab->message, "Saved progress restored. Choose a fresh action.");
    }
  }
  if (input == SELECTED_READY) {
    if (frame >= lab->minimum_action_revision && frame <= lab->revision &&
        !lab->suspended) {
      lab->ready = frame == lab->revision;
      lab->acknowledged_revision = frame;
      lab->acknowledged_interaction_epoch = lab->interaction_epoch;
    }
    return;
  }
  if (input == SELECTED_CANCEL || input == SELECTED_SUSPEND ||
      input == SELECTED_RESUME) {
    memset(lab->gestures, 0, sizeof(lab->gestures));
    if (input != SELECTED_CANCEL) {
      lab->suspended = input == SELECTED_SUSPEND;
      interaction_changed(lab);
    }
    game_rules_resume_runtime(&lab->game, lab->clock);
    return;
  }
  (void)delta;
  if (input > SELECTED_BACK_UP)
    return;
  unsigned button = (unsigned)input / 2;
  SelectedGesture *gesture = &lab->gestures[button];
  if ((unsigned)input % 2 == 0) {
    if (gesture->held)
      return;
    int overlap = selected_lab_held(lab);
    if (overlap)
      for (unsigned i = 0; i < 10; ++i)
        lab->gestures[i].allowed = 0;
    gesture->held = 1;
    gesture->revision = frame;
    gesture->interaction_epoch = lab->interaction_epoch;
    gesture->allowed =
        !overlap && !lab->suspended && !lab->storage_error &&
        frame == lab->acknowledged_revision &&
        lab->acknowledged_interaction_epoch == lab->interaction_epoch;
    return;
  }
  if (!gesture->held)
    return;
  int allowed = gesture->allowed && gesture->revision == frame &&
                gesture->interaction_epoch == lab->interaction_epoch;
  memset(gesture, 0, sizeof(*gesture));
  if (!allowed || lab->suspended)
    return;
  if (button == 0 || button == 1) {
    unsigned count = selected_lab_options(lab);
    lab->focus = (lab->focus + (button == 1 ? 1 : count - 1)) % count;
    if (lab->page == V1_CRITTERS)
      focus_resident(lab);
    interaction_changed(lab);
    return;
  }
  if (button >= 4 && button <= 7) {
    if (button == 5) {
      enter(lab, V1_HOME);
      memset(lab->gestures, 0, sizeof(lab->gestures));
      return;
    }
    remember_workspace(lab);
    unsigned workspace = button - 4;
    enter(lab, lab->workspace_page[workspace]);
    lab->focus = lab->workspace_focus[workspace];
    lab->sample = lab->workspace_sample[workspace];
    lab->study = lab->workspace_study[workspace];
    lab->resident = lab->workspace_resident[workspace];
    if (lab->focus >= selected_lab_options(lab))
      lab->focus = 0;
    if (lab->page == V1_CRITTERS)
      focus_resident(lab);
    else if (lab->page == V1_HABITAT && visible_residents(lab) &&
             !lab->game.individuals[lab->resident].revealed)
      for (unsigned i = 0; i < lab->game.individual_count; ++i)
        if (lab->game.individuals[i].revealed) {
          lab->resident = i;
          break;
        }
    return;
  }
  if (button == 3) {
    int safe = (lab->page == V1_HOME ||
                (lab->page == V1_SAMPLES && lab->game.sample_count) ||
                (lab->page == V1_STUDIES && lab->focus < 5 &&
                 (lab->game.samples[lab->sample].decoded_studies &
                  (1u << lab->focus))) ||
                (lab->page == V1_EXPEDITION && lab->game.expedition_id[0] &&
                 (lab->focus ||
                  (!lab->game.expedition_active &&
                   lab->game.expedition_elapsed >= GAME_EXPEDITION_SECONDS &&
                   game_transfer_available(&lab->game)))) ||
                lab->page == V1_LIBRARY);
    if (safe)
      activate(lab);
    return;
  }
  if (button == 2 || button == 9) {
    SelectedPage previous = lab->page;
    if (previous == V1_LIBRARY_FINDING) {
      enter(lab, V1_LIBRARY);
      lab->focus = lab->library_index;
    } else if (previous == V1_CREATE_REVIEW) {
      enter(lab, V1_CREATE);
      lab->focus = lab->creation_preference;
    } else if (previous == V1_CRITTERS) {
      enter(lab, V1_HABITAT);
      lab->focus = visible_residents(lab) ? 2 : 0;
    } else if (previous == V1_FINDING || previous == V1_STUDY_REVIEW ||
               previous == V1_CREATE) {
      enter(lab, V1_STUDIES);
      lab->focus = previous == V1_CREATE ? 5 : lab->study;
    } else if (previous == V1_STUDIES) {
      enter(lab, V1_SAMPLES);
      lab->focus = lab->sample;
    } else if (previous == V1_DISCARD_REVIEW) {
      enter(lab, V1_CARGO);
      lab->focus = lab->discard_resource + 1;
    } else if (previous == V1_CARGO) {
      enter(lab, V1_EXPEDITION);
      lab->focus = lab->game.expedition_id[0] ? 1 : 0;
    } else {
      enter(lab, V1_HOME);
      lab->focus = previous == V1_EXPEDITION                            ? 1
                   : previous == V1_SAMPLES                             ? 2
                   : previous == V1_INCUBATION || previous == V1_REVEAL ? 3
                   : previous == V1_HABITAT                             ? 4
                                                                        : 0;
    }
  } else
    activate(lab);
}
