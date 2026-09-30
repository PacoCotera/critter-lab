#include "kit.h"
#include "save_bytes.h"
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void refresh(KitView *view, int interaction) {
  ++view->revision;
  if (interaction) {
    ++view->epoch;
    view->minimum_action_revision = view->revision;
  }
}
static void refresh_all(DeviceKit *kit) {
  ++kit->lab->revision;
  ++kit->lab->interaction_epoch;
  kit->lab->minimum_action_revision = kit->lab->revision;
  kit->lab->ready = 0;
  for (unsigned i = 0; i < 10; ++i)
    kit->lab->gestures[i].allowed = 0;
  refresh(&kit->companion, 1);
  refresh(&kit->dock, 1);
}
static void fail(DeviceKit *kit) {
  kit->failed = 1;
  kit->lab->storage_error = 1;
  strcpy(kit->lab->message,
         "Device journal unavailable. Preserve files; reload to recover.");
  strcpy(kit->companion.message, "Storage unavailable. Cargo preserved.");
  refresh_all(kit);
}
static uint32_t checksum(const KitJournal *journal) {
  const unsigned char *bytes = (const unsigned char *)journal;
  uint32_t hash = 2166136261u;
  for (size_t i = sizeof(journal->checksum); i < sizeof(*journal); ++i)
    hash = (hash ^ bytes[i]) * 16777619u;
  return hash;
}
static int persist(DeviceKit *kit) {
  kit->journal.checksum = checksum(&kit->journal);
  if (save_bytes_write_status(kit->journal_path, &kit->journal,
                              sizeof(kit->journal)) != SAVE_BYTES_COMMITTED) {
    fail(kit);
    return 0;
  }
  return 1;
}
static int pending(const DeviceKit *kit) {
  return kit->journal.phase >= KIT_WAITING &&
         kit->journal.phase <= KIT_ACK_PENDING;
}
static const char *source_expedition(const KitJournal *journal) {
  if (!strncmp(journal->haul_id, "haul-", 5)) {
    const char *separator = strchr(journal->haul_id, '/');
    if (separator)
      return !strcmp(separator + 1, "-") ? "" : separator + 1;
  }
  return journal->haul_id;
}
static int same_cargo(const DeviceKit *kit) {
  const GameState *game = &kit->lab->game;
  const KitJournal *journal = &kit->journal;
  return !strcmp(game->expedition_id, source_expedition(journal)) &&
         game->expedition_data == journal->cargo[0] &&
         game->expedition_energy == journal->cargo[1] &&
         game->expedition_essence == journal->cargo[2] &&
         game->expedition_elapsed == journal->elapsed &&
         game->expedition_kind == journal->kind;
}
static GameResult apply(DeviceKit *kit, GameCommand command,
                        const char *identity) {
  char generated[64];
  if (kit->failed || kit->lab->storage_error)
    return GAME_STORAGE;
  if (!command.sequence)
    command.sequence = kit->lab->game.last_operation_sequence + 1;
  snprintf(generated, sizeof(generated), "kit-%llu",
           (unsigned long long)command.sequence);
  command.operation_id = identity ? identity : generated;
  GameResult result =
      game_apply(kit->lab->save_path, &kit->lab->game, &command);
  if (result == GAME_STORAGE || result == GAME_COMMITTED_UNCERTAIN)
    fail(kit);
  if (result == GAME_OK) {
    ++kit->lab->revision;
    refresh(&kit->companion, 1);
  }
  return result;
}
/* A persisted intent reserves the next game sequence. No later command can run
 * until this exact operation is committed/reconciled and its receipt is saved.
 */
static int reconcile(DeviceKit *kit) {
  if (kit->journal.phase != KIT_COMMITTING)
    return 1;
  GameState *game = &kit->lab->game;
  GameCommand command = {0};
  command.type = kit->journal.version == 1 ? GAME_COMMAND_EXPEDITION_OFFLOAD
                 : kit->journal.version == 2
                     ? GAME_COMMAND_EXPEDITION_TRANSFER
                 : kit->journal.version == 3
                     ? GAME_COMMAND_EXPEDITION_WHOLE_TRANSFER
                     : GAME_COMMAND_EXPEDITION_UNLOAD;
  command.sequence = kit->journal.accept_sequence;
  int committed = 0;
  if (game->last_operation_sequence == kit->journal.accept_sequence) {
    for (unsigned i = 0; i < GAME_OPERATION_SLOTS; ++i)
      if (game->operations[i].sequence == kit->journal.accept_sequence &&
          !strcmp(game->operations[i].id, kit->journal.haul_id))
        committed = apply(kit, command, kit->journal.haul_id) == GAME_DUPLICATE;
  } else if (game->last_operation_sequence + 1 ==
                 kit->journal.accept_sequence &&
             same_cargo(kit)) {
    committed = apply(kit, command, kit->journal.haul_id) == GAME_OK;
  }
  if (!committed) {
    fail(kit);
    return 0;
  }
  kit->journal.phase = KIT_ACK_PENDING;
  if (!persist(kit))
    return 0;
  strcpy(kit->lab->message, "Haul accepted. Receipt waiting for Companion.");
  kit->next_delivery = kit->clock + 2;
  refresh_all(kit);
  return 1;
}
static int valid_journal(const KitJournal *journal) {
  if (!memchr(journal->haul_id, 0, sizeof(journal->haul_id)))
    return 0;
  if (!strncmp(journal->haul_id, "haul-", 5)) {
    char *separator = NULL;
    errno = 0;
    unsigned long long sequence =
        strtoull(journal->haul_id + 5, &separator, 10);
    if (errno || !sequence || journal->haul_id[5] < '0' ||
        journal->haul_id[5] > '9' || !separator || *separator != '/' ||
        !separator[1] || strchr(separator + 1, '/'))
      return 0;
  }
  return journal->version >= 1 && journal->version <= 4 &&
         journal->checksum == checksum(journal) &&
         journal->phase <= KIT_COMPLETE && journal->companion_online <= 1 &&
         journal->dock_online <= 1 &&
         (journal->phase == KIT_IDLE || journal->haul_id[0]) &&
         journal->cargo[0] + (uint64_t)journal->cargo[1] + journal->cargo[2] <=
             GAME_CARGO_CAPACITY &&
         journal->elapsed <= GAME_EXPEDITION_SECONDS &&
         journal->kind <= GAME_EXPEDITION_RESONANCE;
}
static void open_reception(DeviceKit *kit) {
  if (kit->journal.phase != KIT_ARRIVED ||
      !strcmp(kit->opened_haul, kit->journal.haul_id))
    return;
  SelectedLab *lab = kit->lab;
  selected_lab_capture_context(lab, &kit->caller);
  kit->caller_valid = 1;
  strcpy(kit->opened_haul, kit->journal.haul_id);
  selected_lab_open_reception(lab);
  refresh(&kit->companion, 1);
  refresh(&kit->dock, 1);
}
static void return_to_caller(DeviceKit *kit) {
  SelectedLab *lab = kit->lab;
  selected_lab_restore_context(lab, &kit->caller);
  kit->caller_valid = 0;
  refresh(&kit->companion, 1);
  refresh(&kit->dock, 1);
}
static int normalize_stock(DeviceKit *kit) {
  kit->normalization_pending = game_supply_conversion_pending(&kit->lab->game);
  if (!kit->normalization_pending || kit->journal.phase == KIT_WAITING ||
      kit->journal.phase == KIT_ARRIVED || kit->journal.phase == KIT_COMMITTING)
    return 1;
  GameCommand command = {0};
  command.type = GAME_COMMAND_STOCK_NORMALIZE;
  GameResult result = apply(kit, command, NULL);
  if (result == GAME_OK)
    kit->normalization_pending = 0;
  return result == GAME_OK || result == GAME_UNAVAILABLE;
}
int kit_init(DeviceKit *kit, SelectedLab *lab, uint32_t clock) {
  memset(kit, 0, sizeof(*kit));
  kit->lab = lab;
  lab->kit_mode = 1;
  kit->clock = clock;
  kit->companion.revision = kit->companion.epoch = 1;
  kit->dock.revision = kit->dock.epoch = 1;
  kit->companion.minimum_action_revision = 1;
  kit->companion.page = COMP_MODES;
  kit->dock.minimum_action_revision = 1;
  snprintf(kit->journal_path, sizeof(kit->journal_path), "%s.kit",
           lab->save_path);
  char marker[580];
  snprintf(marker, sizeof(marker), "%s.required", kit->journal_path);
  FILE *file = fopen(kit->journal_path, "rb");
  if (file) {
    size_t read = fread(&kit->journal, 1, sizeof(kit->journal), file);
    int extra = fgetc(file);
    int closed = fclose(file);
    if (read != sizeof(kit->journal) || extra != EOF || closed ||
        !valid_journal(&kit->journal)) {
      fail(kit);
      return 0;
    }
  } else {
    if (errno != ENOENT) {
      fail(kit);
      return 0;
    }
    file = fopen(marker, "rb");
    if (file) {
      fclose(file);
      fail(kit);
      return 0;
    }
    if (errno != ENOENT) {
      fail(kit);
      return 0;
    }
    kit->journal.version = 4;
    kit->journal.companion_online = kit->journal.dock_online = 1;
    if (!persist(kit))
      return 0;
  }
  if (save_bytes_write(marker, "KIT1", 4)) {
    fail(kit);
    return 0;
  }
  if ((kit->journal.phase == KIT_WAITING ||
       kit->journal.phase == KIT_ARRIVED) &&
      !same_cargo(kit)) {
    fail(kit);
    return 0;
  }
  if (kit->journal.phase >= KIT_ACK_PENDING) {
    if (!kit->journal.accept_sequence ||
        lab->game.last_operation_sequence < kit->journal.accept_sequence) {
      fail(kit);
      return 0;
    }
    int found = 0;
    for (unsigned i = 0; i < GAME_OPERATION_SLOTS; ++i) {
      const GameOperation *operation = &lab->game.operations[i];
      if (operation->sequence == kit->journal.accept_sequence)
        found = 1;
      if (operation->sequence == kit->journal.accept_sequence &&
          strcmp(operation->id, kit->journal.haul_id)) {
        fail(kit);
        return 0;
      }
    }
    if (lab->game.last_operation_sequence == kit->journal.accept_sequence &&
        !found) {
      fail(kit);
      return 0;
    }
  }
  if (kit->journal.phase == KIT_ACK_PENDING &&
      (lab->game.expedition_active ||
       (kit->journal.version == 1 || kit->journal.version >= 4
            ? lab->game.expedition_id[0] != 0 ||
                  lab->game.expedition_elapsed != 0
        : kit->journal.elapsed < GAME_EXPEDITION_SECONDS
            ? strcmp(lab->game.expedition_id,
                     source_expedition(&kit->journal)) ||
                  lab->game.expedition_elapsed != kit->journal.elapsed ||
                  lab->game.expedition_kind != kit->journal.kind
            : lab->game.expedition_id[0] != 0 ||
                  lab->game.expedition_elapsed != 0))) {
    fail(kit);
    return 0;
  }
  kit->next_delivery = clock + 2;
  if (!reconcile(kit) || !normalize_stock(kit))
    return 0;
  open_reception(kit);
  return 1;
}
const char *kit_stage(const DeviceKit *kit) {
  if (kit->failed || kit->lab->storage_error)
    return "Storage unavailable";
  if (kit->journal.phase == KIT_WAITING)
    return kit->journal.companion_online ? "Sending to Lab"
                                         : "Waiting for Lab link";
  static const char *names[] = {"",
                                "",
                                "Received at Lab - accept there",
                                "Saving in Lab",
                                "Accepted in Lab - receipt pending",
                                "Receipt confirmed"};
  return names[kit->journal.phase];
}
const char *kit_route(const DeviceKit *kit) {
  static const char *routes[] = {"Field survey", "Garden forage",
                                 "Weather watch"};
  if (pending(kit) && !source_expedition(&kit->journal)[0])
    return "Stored supplies";
  unsigned kind =
      pending(kit) ? kit->journal.kind : kit->lab->game.expedition_kind;
  return routes[kind % 3];
}
const char *kit_expedition_status(const DeviceKit *kit) {
  const GameState *game = &kit->lab->game;
  if (kit->journal.phase >= KIT_ACK_PENDING &&
      !game->expedition_id[0] && source_expedition(&kit->journal)[0])
    return "Expedition ended";
  if (pending(kit))
    return "Returning";
  if (game->expedition_elapsed >= GAME_EXPEDITION_SECONDS)
    return "Expedition complete";
  if (kit->companion.page == COMP_SEND_REVIEW ||
      (game->expedition_id[0] && !game->expedition_active))
    return "Paused";
  if (game->expedition_active) {
    return game_gather_capacity_blocked(game) ? "Not enough room" : "Gathering";
  }
  return "Choose a route";
}
const GameSample *kit_received_sample(const DeviceKit *kit) {
  if (kit->journal.phase < KIT_ACK_PENDING)
    return NULL;
  for (unsigned i = 0; i < kit->lab->game.sample_count; ++i)
    if (!strcmp(kit->lab->game.samples[i].origin_expedition_id,
                source_expedition(&kit->journal)))
      return &kit->lab->game.samples[i];
  return NULL;
}
int kit_lab_explore(const DeviceKit *kit) {
  return kit->lab->page == V1_EXPEDITION || kit->lab->page == V1_CARGO;
}
static unsigned probe_option_count(const DeviceKit *kit) {
  if (!pending(kit) && kit->lab->game.expedition_id[0] &&
      !game_transfer_available(&kit->lab->game))
    return 2;
  return kit->lab->game.expedition_id[0] || pending(kit) ? 1 : 3;
}
unsigned kit_option_count(const DeviceKit *kit, unsigned device) {
  if (device == KIT_DOCK)
    return kit->dock.page == 2 ? 2 : 3;
  switch (kit->companion.page) {
  case COMP_MODES:
    return 3;
  case COMP_SEND_REVIEW:
    return 2;
  case COMP_CARGO:
    return 1;
  case COMP_FRIENDS:
    return 0;
  default:
    return probe_option_count(kit);
  }
}
const char *kit_option(const DeviceKit *kit, unsigned device, unsigned index) {
  if (device == KIT_DOCK) {
    if (kit->dock.page == 2)
      return index ? "Cancel" : "Print preview (simulation)";
    static const char *pages[] = {"World", "Supplies", "Connections"};
    return pages[index % 3];
  }
  switch (kit->companion.page) {
  case COMP_MODES: {
    static const char *modes[] = {"Probe", "Cargo", "Companions"};
    return modes[index % 3];
  }
  case COMP_SEND_REVIEW:
    return index ? "Keep cargo" : "Send to Lab";
  case COMP_CARGO:
    return pending(kit)                               ? "View expedition"
           : game_transfer_available(&kit->lab->game) ? "Send to Lab"
                                                      : "Return to Probe";
  case COMP_FRIENDS:
    return "Modes";
  default: {
    static const char *routes[] = {"Field survey", "Garden forage",
                                   "Weather watch"};
    if (!pending(kit) && kit->lab->game.expedition_id[0] &&
        !game_transfer_available(&kit->lab->game))
      return index ? "Finish expedition" : "View cargo";
    return kit->lab->game.expedition_id[0] || pending(kit) ? "View cargo"
                                                           : routes[index % 3];
  }
  }
}
static void companion_page(DeviceKit *kit, unsigned page) {
  kit->companion.page = page;
  kit->companion.focus = 0;
  if (page < COMP_MODES)
    kit->companion.mode = page;
  refresh(&kit->companion, 1);
}
static void companion_task(DeviceKit *kit, unsigned page) {
  KitView *view = &kit->companion;
  if (view->task_depth < 2) {
    view->task_page[view->task_depth] = view->page;
    view->task_focus[view->task_depth] = view->focus;
    ++view->task_depth;
  }
  companion_page(kit, page);
}
static void companion_back(DeviceKit *kit) {
  KitView *view = &kit->companion;
  if (view->task_depth) {
    --view->task_depth;
    companion_page(kit, view->task_page[view->task_depth]);
    unsigned count = kit_option_count(kit, KIT_COMPANION);
    unsigned remembered = view->task_focus[view->task_depth];
    view->focus = count && remembered < count ? remembered : 0;
  } else if (view->page != COMP_MODES) {
    if (view->page < COMP_MODES) {
      view->mode = view->page;
      view->action_focus[view->mode] = view->focus;
    }
    view->page = COMP_MODES;
    view->focus = view->mode;
    refresh(view, 1);
  }
}
static void companion_enter_actions(DeviceKit *kit) {
  KitView *view = &kit->companion;
  if (view->mode == COMP_FRIENDS)
    return;
  companion_page(kit, view->mode);
  unsigned count = kit_option_count(kit, KIT_COMPANION);
  unsigned remembered = view->action_focus[view->mode];
  view->focus = count && remembered < count ? remembered : 0;
}
static void seal(DeviceKit *kit) {
  GameState *game = &kit->lab->game;
  if (pending(kit) || !game_transfer_available(game))
    return;
  char identity[64];
  int length = snprintf(identity, sizeof(identity), "haul-%llu/%s",
                        (unsigned long long)(game->last_operation_sequence + 1),
                        game->expedition_id[0] ? game->expedition_id : "-");
  if (length < 0 || length >= (int)sizeof(identity)) {
    strcpy(kit->companion.message,
           "Transfer identity is too long. Cargo kept.");
    refresh(&kit->companion, 1);
    return;
  }
  kit->journal.version = 4;
  strcpy(kit->journal.haul_id, identity);
  kit->journal.cargo[0] = game->expedition_data;
  kit->journal.cargo[1] = game->expedition_energy;
  kit->journal.cargo[2] = game->expedition_essence;
  kit->journal.elapsed = game->expedition_elapsed;
  kit->journal.kind = game->expedition_kind;
  kit->journal.phase = KIT_WAITING;
  kit->journal.accept_sequence = 0;
  if (!persist(kit))
    return;
  kit->next_delivery = kit->clock + 2;
  companion_back(kit);
  refresh_all(kit);
}
static void activate_companion(DeviceKit *kit) {
  unsigned focus = kit->companion.focus;
  switch (kit->companion.page) {
  case COMP_MODES:
    companion_enter_actions(kit);
    break;
  case COMP_SEND_REVIEW:
    if (focus)
      companion_back(kit);
    else
      seal(kit);
    break;
  case COMP_CARGO:
    if (pending(kit)) {
      if (kit->companion.task_depth)
        companion_back(kit);
      else
        companion_page(kit, COMP_PROBE);
    } else if (game_transfer_available(&kit->lab->game))
      companion_task(kit, COMP_SEND_REVIEW);
    else {
      companion_page(kit, COMP_PROBE);
      strcpy(kit->companion.message, "Ready to gather. Choose an expedition.");
    }
    break;
  case COMP_FRIENDS:
    companion_page(kit, COMP_MODES);
    break;
  default:
    if (!pending(kit) && kit->lab->game.expedition_id[0] &&
        focus == 1 &&
        !game_transfer_available(&kit->lab->game)) {
      GameCommand command = {0};
      command.type = GAME_COMMAND_EXPEDITION_FINISH;
      if (apply(kit, command, NULL) == GAME_OK) {
        strcpy(kit->companion.message, "Expedition ended. Choose a new route.");
        refresh_all(kit);
      }
      break;
    }
    if (kit->lab->game.expedition_id[0] || pending(kit))
      companion_task(kit, COMP_CARGO);
    else {
      GameCommand command = {0};
      command.type = GAME_COMMAND_EXPEDITION_START;
      command.data.expedition.kind = (GameExpeditionKind)focus;
      command.data.expedition.monotonic_seconds = kit->clock;
      if (apply(kit, command, NULL) == GAME_OK) {
        if (kit->journal.phase == KIT_COMPLETE) {
          kit->journal.phase = KIT_IDLE;
          persist(kit);
        }
        kit->companion.focus = 0;
        strcpy(kit->companion.message,
               "Gathering on Companion. Cargo stays here.");
      }
    }
  }
}
static void accept(DeviceKit *kit) {
  if (kit->journal.phase != KIT_ARRIVED || !same_cargo(kit))
    return;
  kit->journal.accept_sequence = kit->lab->game.last_operation_sequence + 1;
  /* A preacceptance snapshot stays intact; only this fresh intent adopts the
   * ending policy. Already reserved v1/v2/v3 intents keep their exact command. */
  kit->journal.version = 4;
  kit->journal.phase = KIT_COMMITTING;
  if (persist(kit) && reconcile(kit))
    normalize_stock(kit);
}
static int held(const KitView *view) {
  for (unsigned i = 0; i < 10; ++i)
    if (view->gestures[i].held)
      return 1;
  return 0;
}
void kit_input(DeviceKit *kit, unsigned device, SelectedInput input,
               unsigned revision) {
  if (device == KIT_LAB) {
    if (kit->failed || kit->journal.phase == KIT_COMMITTING)
      return;
    if (kit_lab_explore(kit) &&
        (input == SELECTED_CONFIRM_UP ||
         ((input == SELECTED_BACK_UP || input == SELECTED_LEFT_UP) &&
          kit->caller_valid))) {
      unsigned button = (unsigned)input / 2;
      SelectedGesture before = kit->lab->gestures[button];
      int allowed = before.held && before.allowed &&
                    before.revision == revision &&
                    before.interaction_epoch == kit->lab->interaction_epoch &&
                    !kit->lab->suspended;
      memset(&kit->lab->gestures[button], 0, sizeof(SelectedGesture));
      if (allowed) {
        if (input == SELECTED_CONFIRM_UP)
          accept(kit);
        else
          return_to_caller(kit);
      }
    } else
      selected_lab_input(kit->lab, input, 0, revision);
    return;
  }
  KitView *view = device == KIT_COMPANION ? &kit->companion : &kit->dock;
  if (input == SELECTED_READY) {
    if (!view->suspended && revision >= view->minimum_action_revision &&
        revision <= view->revision) {
      view->acknowledged = revision;
      view->acknowledged_epoch = view->epoch;
    }
    return;
  }
  if (input >= SELECTED_CANCEL) {
    memset(view->gestures, 0, sizeof(view->gestures));
    if (input != SELECTED_CANCEL) {
      view->suspended = input == SELECTED_SUSPEND;
      refresh(view, 1);
    }
    return;
  }
  unsigned button = (unsigned)input / 2;
  if (button >= 10)
    return;
  SelectedGesture *gesture = &view->gestures[button];
  if ((unsigned)input % 2 == 0) {
    if (gesture->held)
      return;
    int overlap = held(view);
    if (overlap)
      for (unsigned i = 0; i < 10; ++i)
        view->gestures[i].allowed = 0;
    gesture->held = 1;
    gesture->revision = revision;
    gesture->interaction_epoch = view->epoch;
    gesture->allowed = !overlap && !view->suspended && !kit->failed &&
                       !kit->lab->storage_error &&
                       revision == view->acknowledged &&
                       view->acknowledged_epoch == view->epoch;
    if (gesture->allowed)
      refresh(view, 0);
    return;
  }
  int allowed = gesture->held && gesture->allowed &&
                gesture->revision == revision &&
                gesture->interaction_epoch == view->epoch;
  memset(gesture, 0, sizeof(*gesture));
  if (!allowed || view->suspended)
    return;
  refresh(view, 0);
  view->message[0] = 0;
  if (device == KIT_COMPANION && view->page == COMP_MODES) {
    if (button == 2 || button == 3) {
      unsigned mode = view->mode;
      if (button == 2 && mode)
        --mode;
      if (button == 3 && mode < COMP_FRIENDS)
        ++mode;
      if (mode != view->mode) {
        view->mode = view->focus = mode;
        refresh(view, 1);
      }
    } else if (button == 1 || button == 8)
      companion_enter_actions(kit);
    return;
  }
  if (button <= 1) {
    unsigned count = kit_option_count(kit, device);
    if (!count)
      return;
    if (device == KIT_COMPANION) {
      if (!button && !view->focus && !view->task_depth) {
        companion_back(kit);
        return;
      }
      unsigned focus = view->focus;
      if (!button && focus)
        --focus;
      if (button && focus + 1 < count)
        ++focus;
      if (focus != view->focus) {
        view->focus = focus;
        if (view->page < COMP_MODES)
          view->action_focus[view->page] = focus;
        refresh(view, 1);
      }
    } else {
      view->focus = (view->focus + (button ? 1 : count - 1)) % count;
      refresh(view, 1);
    }
  } else if (device == KIT_COMPANION) {
    if (button == 9)
      companion_back(kit);
    else if (button == 8)
      activate_companion(kit);
  } else if (button == 4) {
    view->page = 2;
    view->focus = 0;
    refresh(view, 1);
  } else if (button == 5) {
    strcpy(view->message, "Feed simulated. No physical printer.");
    refresh(view, 0);
  } else if (button == 8) {
    if (view->page == 2) {
      strcpy(view->message, view->focus
                                ? "Print cancelled"
                                : "Preview only. Printer not connected.");
      view->page = 0;
      view->focus = 0;
    } else
      view->page = view->page ? 0 : 1;
    refresh(view, 1);
  }
}
int kit_link(DeviceKit *kit, unsigned device, int online) {
  if (kit->failed || (device != KIT_COMPANION && device != KIT_DOCK))
    return 0;
  if (device == KIT_COMPANION)
    kit->journal.companion_online = online != 0;
  else
    kit->journal.dock_online = online != 0;
  if (!persist(kit))
    return 0;
  kit->next_delivery = kit->clock + 2;
  refresh_all(kit);
  return 1;
}
void kit_tick(DeviceKit *kit, uint32_t clock) {
  kit->clock = clock;
  if (kit->failed || kit->lab->storage_error || !reconcile(kit) ||
      !normalize_stock(kit))
    return;
  uint64_t before = kit->lab->game.revision;
  int before_active = kit->lab->game.expedition_active;
  int before_transfer = game_transfer_available(&kit->lab->game);
  int before_empty =
      !(kit->lab->game.expedition_data + kit->lab->game.expedition_energy +
        kit->lab->game.expedition_essence);
  int portable_held = held(&kit->companion);
  int expedition = !pending(kit) && !portable_held &&
                   !kit->companion.suspended &&
                   kit->companion.page != COMP_SEND_REVIEW;
  if (!expedition)
    kit->lab->game.expedition_last_tick = clock;
  selected_lab_tick_devices(kit->lab, clock, expedition, 1);
  if (kit->lab->game.revision != before) {
    unsigned count = kit_option_count(kit, KIT_COMPANION);
    int focus_changed = kit->companion.focus >= count &&
                        kit->companion.focus != 0;
    if (focus_changed)
      kit->companion.focus = count ? count - 1 : 0;
    unsigned probe_count = probe_option_count(kit);
    if (kit->companion.action_focus[COMP_PROBE] >= probe_count)
      kit->companion.action_focus[COMP_PROBE] = probe_count - 1;
    int after_empty =
        !(kit->lab->game.expedition_data + kit->lab->game.expedition_energy +
          kit->lab->game.expedition_essence);
    refresh(&kit->companion,
            focus_changed ||
                before_active != kit->lab->game.expedition_active ||
                before_empty != after_empty ||
                before_transfer != game_transfer_available(&kit->lab->game));
  }
  if (kit->journal.companion_online && clock >= kit->next_delivery) {
    if (kit->journal.phase == KIT_WAITING) {
      kit->journal.phase = KIT_ARRIVED;
      if (!persist(kit))
        return;
      open_reception(kit);
    } else if (kit->journal.phase == KIT_ACK_PENDING) {
      kit->journal.phase = KIT_COMPLETE;
      if (!persist(kit))
        return;
      strcpy(kit->companion.message,
             "Receipt confirmed. Choose a new expedition.");
      strcpy(kit->lab->message, "Haul accepted; Companion receipt confirmed.");
      refresh_all(kit);
    }
  }
  if (kit->journal.dock_online) {
    const GameState *game = &kit->lab->game;
    unsigned residents = 0;
    for (unsigned i = 0; i < game->individual_count; ++i)
      residents += game->individuals[i].revealed;
    int changed = kit->journal.dock_stock[0] != game->data ||
                  kit->journal.dock_stock[1] != game->energy ||
                  kit->journal.dock_stock[2] != game->essence ||
                  kit->journal.dock_samples != game->sample_count ||
                  kit->journal.dock_residents != residents ||
                  kit->journal.dock_incubations != game->incubation_active;
    if (changed || !kit->dock_updated) {
      kit->journal.dock_stock[0] = game->data;
      kit->journal.dock_stock[1] = game->energy;
      kit->journal.dock_stock[2] = game->essence;
      kit->journal.dock_samples = game->sample_count;
      kit->journal.dock_residents = residents;
      kit->journal.dock_incubations = game->incubation_active;
      kit->journal.dock_world_revision = game->revision;
      kit->dock_updated = clock;
      kit->journal.dock_updated_at = (uint64_t)time(NULL);
      if (!persist(kit))
        return;
      refresh(&kit->dock, 0);
    }
  }
}
unsigned kit_revision(const DeviceKit *kit, unsigned device) {
  return device == KIT_LAB         ? kit->lab->revision
         : device == KIT_COMPANION ? kit->companion.revision
                                   : kit->dock.revision;
}
unsigned kit_width(unsigned device) {
  return device == KIT_LAB ? 1024 : device == KIT_COMPANION ? 450 : 792;
}
unsigned kit_height(unsigned device) { return device == KIT_DOCK ? 272 : 600; }
void kit_status(DeviceKit *kit, unsigned device, FILE *output) {
  const KitView *view = device == KIT_COMPANION ? &kit->companion : &kit->dock;
  const GameState *game = &kit->lab->game;
  const char *focus =
      device == KIT_LAB
          ? (kit_lab_explore(kit)
                 ? (kit->journal.phase == KIT_ARRIVED ? "Accept haul"
                                                      : "Companion expeditions")
                 : selected_lab_focus(kit->lab))
          : kit_option(kit, device, view->focus);
  const char *page = device == KIT_LAB          ? selected_lab_page(kit->lab)
                     : device == KIT_DOCK       ? "dock"
                     : view->page == COMP_PROBE ? "probe"
                     : view->page == COMP_CARGO ? "cargo"
                     : view->page == COMP_MODES ? "modes"
                     : view->page == COMP_SEND_REVIEW ? "send-review"
                                                      : "companions";
  int online = device == KIT_COMPANION ? kit->journal.companion_online
               : device == KIT_DOCK    ? kit->journal.dock_online
                                       : 1;
  const uint32_t *cargo =
      kit->journal.phase >= KIT_WAITING && kit->journal.phase <= KIT_COMMITTING
          ? kit->journal.cargo
          : NULL;
  fprintf(
      output,
      "{\"device\":%u,\"revision\":%u,\"width\":%u,\"height\":%u,\"page\":"
      "\"%s\",\"focus\":\"%s\",\"workspace\":%u,\"online\":%s,\"transfer\":"
      "\"%s\",\"phase\":%u,\"haul\":\"%s\",\"stock\":[%u,%u,%u],\"cargo\":["
      "%u,%u,%u],\"samples\":%u,\"residents\":%u,\"dock_stock\":[%u,%u,%u],"
      "\"dock_cached\":%s,\"failed\":%s,\"stock_conversion_pending\":%s,"
      "\"mode\":%u,\"expedition_seconds\":%u,\"gather_progress_ms\":[%u,%u,%u],"
      "\"gather_remaining_ms\":%u,\"gather_attempted\":%u,\"gather_awarded\":%"
      "u,"
      "\"boundary\":\"Simulated wireless; "
      "radio not selected\"}\n",
      device, kit_revision(kit, device), kit_width(device), kit_height(device),
      page, focus, kit->lab->workspace, online ? "true" : "false",
      device == KIT_LAB && kit->journal.phase == KIT_ACK_PENDING
          ? "Haul accepted; Companion receipt pending"
          : kit_stage(kit),
      kit->journal.phase, kit->journal.haul_id, game->data, game->energy,
      game->essence, cargo ? cargo[0] : game->expedition_data,
      cargo ? cargo[1] : game->expedition_energy,
      cargo ? cargo[2] : game->expedition_essence, game->sample_count,
      kit->journal.dock_residents, kit->journal.dock_stock[0],
      kit->journal.dock_stock[1], kit->journal.dock_stock[2],
      kit->journal.dock_online ? "false" : "true",
      kit->failed ? "true" : "false",
      kit->normalization_pending ? "true" : "false", kit->companion.mode,
      game->expedition_elapsed, game->gather_progress_ms[0],
      game->gather_progress_ms[1], game->gather_progress_ms[2],
      game_gather_remaining_ms(game), game->gather_last_attempted_mask,
      game->gather_last_awarded_mask);
}
