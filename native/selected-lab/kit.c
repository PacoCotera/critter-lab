#include "kit.h"
#include "save_bytes.h"
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <time.h>

static void refresh(KitView *view, int interaction) {
  ++view->revision;
  if (interaction)
    ++view->epoch;
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
static int same_cargo(const DeviceKit *kit) {
  const GameState *game = &kit->lab->game;
  const KitJournal *journal = &kit->journal;
  return !strcmp(game->expedition_id, journal->haul_id) &&
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
  int committed = 0;
  if (game->last_operation_sequence == kit->journal.accept_sequence) {
    for (unsigned i = 0; i < GAME_OPERATION_SLOTS; ++i)
      if (game->operations[i].sequence == kit->journal.accept_sequence &&
          !strcmp(game->operations[i].id, kit->journal.haul_id))
        committed = 1;
  } else if (game->last_operation_sequence + 1 ==
                 kit->journal.accept_sequence &&
             same_cargo(kit)) {
    GameCommand command = {0};
    command.type = GAME_COMMAND_EXPEDITION_OFFLOAD;
    command.sequence = kit->journal.accept_sequence;
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
  return journal->version == 1 && journal->checksum == checksum(journal) &&
         journal->phase <= KIT_COMPLETE && journal->companion_online <= 1 &&
         journal->dock_online <= 1 &&
         memchr(journal->haul_id, 0, sizeof(journal->haul_id)) &&
         (journal->phase == KIT_IDLE || journal->haul_id[0]) &&
         journal->cargo[0] + (uint64_t)journal->cargo[1] + journal->cargo[2] <=
             GAME_CARGO_CAPACITY &&
         journal->elapsed <= GAME_EXPEDITION_SECONDS &&
         journal->kind <= GAME_EXPEDITION_RESONANCE;
}
int kit_init(DeviceKit *kit, SelectedLab *lab, uint32_t clock) {
  memset(kit, 0, sizeof(*kit));
  kit->lab = lab;
  lab->kit_mode = 1;
  kit->clock = clock;
  kit->companion.revision = kit->companion.epoch = 1;
  kit->dock.revision = kit->dock.epoch = 1;
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
    kit->journal.version = 1;
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
    for (unsigned i = 0; i < GAME_OPERATION_SLOTS; ++i) {
      const GameOperation *operation = &lab->game.operations[i];
      if (operation->sequence == kit->journal.accept_sequence &&
          strcmp(operation->id, kit->journal.haul_id)) {
        fail(kit);
        return 0;
      }
    }
  }
  if (kit->journal.phase == KIT_ACK_PENDING && lab->game.expedition_id[0]) {
    fail(kit);
    return 0;
  }
  kit->next_delivery = clock + 2;
  return reconcile(kit);
}
const char *kit_stage(const DeviceKit *kit) {
  if (kit->failed || kit->lab->storage_error)
    return "Storage unavailable";
  static const char *names[] = {
      "No transfer",    "Waiting to send",  "Awaiting Lab acceptance",
      "Saving receipt", "Awaiting receipt", "Transferred"};
  return names[kit->journal.phase];
}
int kit_lab_explore(const DeviceKit *kit) {
  return kit->lab->page == V1_EXPEDITION || kit->lab->page == V1_CARGO;
}
static unsigned options(const DeviceKit *kit, unsigned device) {
  if (device == KIT_DOCK)
    return kit->dock.page == 2 ? 2 : 3;
  switch (kit->companion.page) {
  case COMP_MODES:
    return 3;
  case COMP_SEND_REVIEW:
    return 2;
  case COMP_CARGO:
    return 2;
  case COMP_FRIENDS:
    return 1;
  default:
    return kit->lab->game.expedition_id[0] || pending(kit) ? 2 : 3;
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
    return index ? "Keep gathering" : "Seal and send haul";
  case COMP_CARGO:
    return index ? "Modes" : pending(kit) ? "Check receipt" : "Send haul";
  case COMP_FRIENDS:
    return "Modes";
  default: {
    static const char *routes[] = {"Field survey", "Garden forage",
                                   "Weather watch"};
    return kit->lab->game.expedition_id[0] || pending(kit)
               ? (index ? "Modes" : "View cargo")
               : routes[index % 3];
  }
  }
}
static void companion_page(DeviceKit *kit, unsigned page) {
  kit->companion.page = page;
  kit->companion.focus = 0;
  refresh(&kit->companion, 1);
}
static void seal(DeviceKit *kit) {
  GameState *game = &kit->lab->game;
  if (pending(kit) || !game->expedition_id[0] ||
      !(game->expedition_data + game->expedition_energy +
        game->expedition_essence))
    return;
  strcpy(kit->journal.haul_id, game->expedition_id);
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
  companion_page(kit, COMP_CARGO);
  refresh_all(kit);
}
static void activate_companion(DeviceKit *kit) {
  unsigned focus = kit->companion.focus;
  switch (kit->companion.page) {
  case COMP_MODES:
    companion_page(kit, focus);
    break;
  case COMP_SEND_REVIEW:
    if (focus)
      companion_page(kit, COMP_CARGO);
    else
      seal(kit);
    break;
  case COMP_CARGO:
    if (focus)
      companion_page(kit, COMP_MODES);
    else if (pending(kit)) {
      snprintf(kit->companion.message, sizeof(kit->companion.message), "%s",
               kit_stage(kit));
      refresh(&kit->companion, 0);
    } else if (kit->lab->game.expedition_data +
               kit->lab->game.expedition_energy +
               kit->lab->game.expedition_essence)
      companion_page(kit, COMP_SEND_REVIEW);
    else {
      strcpy(kit->companion.message, "No cargo. Choose a Probe expedition.");
      refresh(&kit->companion, 0);
    }
    break;
  case COMP_FRIENDS:
    companion_page(kit, COMP_MODES);
    break;
  default:
    if (kit->lab->game.expedition_id[0] || pending(kit))
      companion_page(kit, focus ? COMP_MODES : COMP_CARGO);
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
  kit->journal.phase = KIT_COMMITTING;
  if (persist(kit))
    reconcile(kit);
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
    if (kit_lab_explore(kit) && input == SELECTED_CONFIRM_UP) {
      SelectedGesture before = kit->lab->gestures[8];
      int allowed = before.held && before.allowed &&
                    before.revision == revision &&
                    before.interaction_epoch == kit->lab->interaction_epoch &&
                    !kit->lab->suspended;
      selected_lab_input(kit->lab, input, 0, revision);
      if (allowed)
        accept(kit);
    } else
      selected_lab_input(kit->lab, input, 0, revision);
    return;
  }
  KitView *view = device == KIT_COMPANION ? &kit->companion : &kit->dock;
  if (input == SELECTED_READY) {
    if (!view->suspended && revision == view->revision) {
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
    return;
  }
  int allowed = gesture->held && gesture->allowed &&
                gesture->revision == revision &&
                gesture->interaction_epoch == view->epoch;
  memset(gesture, 0, sizeof(*gesture));
  if (!allowed || view->suspended)
    return;
  view->message[0] = 0;
  if (button <= 1) {
    unsigned count = options(kit, device);
    view->focus = (view->focus + (button ? 1 : count - 1)) % count;
    refresh(view, 1);
  } else if (device == KIT_COMPANION) {
    if (button == 9 || button == 2)
      companion_page(kit, COMP_MODES);
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
  if (kit->failed || kit->lab->storage_error || !reconcile(kit))
    return;
  uint64_t before = kit->lab->game.revision;
  int before_active = kit->lab->game.expedition_active;
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
    int after_empty =
        !(kit->lab->game.expedition_data + kit->lab->game.expedition_energy +
          kit->lab->game.expedition_essence);
    refresh(&kit->companion,
            before_active != kit->lab->game.expedition_active ||
                before_empty != after_empty);
  }
  if (kit->journal.companion_online && clock >= kit->next_delivery) {
    if (kit->journal.phase == KIT_WAITING) {
      kit->journal.phase = KIT_ARRIVED;
      if (!persist(kit))
        return;
      strcpy(kit->lab->message,
             "Companion haul waiting in Explore. Review and accept.");
      refresh_all(kit);
    } else if (kit->journal.phase == KIT_ACK_PENDING) {
      kit->journal.phase = KIT_COMPLETE;
      if (!persist(kit))
        return;
      strcpy(kit->companion.message, "Lab receipt saved. Cargo cleared.");
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
  const uint32_t *cargo = pending(kit) ? kit->journal.cargo : NULL;
  fprintf(output,
          "{\"device\":%u,\"revision\":%u,\"width\":%u,\"height\":%u,\"page\":"
          "\"%s\",\"focus\":\"%s\",\"workspace\":%u,\"online\":%s,\"transfer\":"
          "\"%s\",\"phase\":%u,\"haul\":\"%s\",\"stock\":[%u,%u,%u],\"cargo\":["
          "%u,%u,%u],\"samples\":%u,\"residents\":%u,\"dock_stock\":[%u,%u,%u],"
          "\"dock_cached\":%s,\"failed\":%s,\"boundary\":\"Simulated wireless; "
          "radio not selected\"}\n",
          device, kit_revision(kit, device), kit_width(device),
          kit_height(device), page, focus, kit->lab->workspace,
          online ? "true" : "false", kit_stage(kit), kit->journal.phase,
          kit->journal.haul_id, game->data, game->energy, game->essence,
          cargo ? cargo[0] : game->expedition_data,
          cargo ? cargo[1] : game->expedition_energy,
          cargo ? cargo[2] : game->expedition_essence, game->sample_count,
          kit->journal.dock_residents, kit->journal.dock_stock[0],
          kit->journal.dock_stock[1], kit->journal.dock_stock[2],
          kit->journal.dock_online ? "false" : "true",
          kit->failed ? "true" : "false");
}
