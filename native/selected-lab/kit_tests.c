#define _POSIX_C_SOURCE 200809L
#include "kit.h"
#include "save_bytes.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void press(DeviceKit *kit, unsigned device, SelectedInput down) {
  unsigned revision = kit_revision(kit, device);
  kit_input(kit, device, SELECTED_READY, revision);
  kit_input(kit, device, down, revision);
  kit_input(kit, device, (SelectedInput)(down + 1), revision);
}
/* Write a crash-boundary fixture from a real sealed journal. This does not
 * execute a parallel transfer implementation. */
static void persist_fixture(DeviceKit *kit) {
  const unsigned char *bytes = (const unsigned char *)&kit->journal;
  uint32_t hash = 2166136261u;
  for (size_t i = sizeof(kit->journal.checksum); i < sizeof(kit->journal); ++i)
    hash = (hash ^ bytes[i]) * 16777619u;
  kit->journal.checksum = hash;
  assert(save_bytes_write(kit->journal_path, &kit->journal,
                          sizeof(kit->journal)) == 0);
}
int main(void) {
  char directory[] = "/tmp/beecho-kit-XXXXXX";
  assert(mkdtemp(directory));
  char path[512];
  snprintf(path, sizeof(path), "%s/game", directory);
  SelectedLab lab;
  selected_lab_init(&lab);
  assert(selected_lab_load(&lab, path, 100));
  DeviceKit kit;
  assert(kit_init(&kit, &lab, 100));
  unsigned painted_dock = kit_revision(&kit, KIT_DOCK);
  kit_tick(&kit, 100);
  /* A timestamp/cache repaint may occur while an already decoded frame is
   * being acknowledged. It does not change the action meaning. */
  assert(kit_revision(&kit, KIT_DOCK) > painted_dock);
  kit_input(&kit, KIT_DOCK, SELECTED_READY, painted_dock);
  kit_input(&kit, KIT_DOCK, SELECTED_DOWN_DOWN, painted_dock);
  kit_input(&kit, KIT_DOCK, SELECTED_DOWN_UP, painted_dock);
  assert(kit.dock.focus == 1);
  /* Navigation changes the interaction: the previous frame cannot be made
   * eligible again by a late READY or by a fresh down/up pair. */
  kit_input(&kit, KIT_DOCK, SELECTED_READY, painted_dock);
  kit_input(&kit, KIT_DOCK, SELECTED_DOWN_DOWN, painted_dock);
  kit_input(&kit, KIT_DOCK, SELECTED_DOWN_UP, painted_dock);
  assert(kit.dock.focus == 1);
  press(&kit, KIT_DOCK, SELECTED_UP_DOWN);
  assert(kit.dock.focus == 0);
  /* Separate native sizes and true binary monochrome, including padded BMP
   * rows. */
  for (unsigned device = 0; device < 3; ++device) {
    FILE *frame = tmpfile();
    assert(frame);
    assert(kit_bmp(&kit, device, frame));
    assert(ftell(frame) == 54 + (long)(((kit_width(device) * 3 + 3) & ~3u) *
                                       kit_height(device)));
    if (device == KIT_DOCK) {
      rewind(frame);
      assert(fseek(frame, 54, SEEK_SET) == 0);
      int value;
      while ((value = fgetc(frame)) != EOF)
        assert(value == 0 || value == 255);
    }
    fclose(frame);
  }
  /* Lab cannot start an expedition. */
  press(&kit, KIT_LAB, SELECTED_DOWN_DOWN);
  press(&kit, KIT_LAB, SELECTED_CONFIRM_DOWN);
  press(&kit, KIT_LAB, SELECTED_CONFIRM_DOWN);
  assert(!lab.game.expedition_id[0]);
  /* Companion runs the route and keeps cargo separate from spendable Lab stock.
   */
  press(&kit, KIT_COMPANION, SELECTED_CONFIRM_DOWN);
  assert(lab.game.expedition_active);
  kit_tick(&kit, 110);
  unsigned painted_companion = kit_revision(&kit, KIT_COMPANION);
  unsigned painted_epoch = kit.companion.epoch;
  kit_tick(&kit, 111);
  assert(kit_revision(&kit, KIT_COMPANION) > painted_companion &&
         kit.companion.epoch == painted_epoch);
  kit_input(&kit, KIT_COMPANION, SELECTED_READY, painted_companion);
  kit_input(&kit, KIT_COMPANION, SELECTED_DOWN_DOWN, painted_companion);
  kit_input(&kit, KIT_COMPANION, SELECTED_DOWN_UP, painted_companion);
  assert(kit.companion.focus == 1);
  press(&kit, KIT_COMPANION, SELECTED_UP_DOWN);
  kit_tick(&kit, 160);
  assert(lab.game.expedition_elapsed == 60 && lab.game.data == 0);
  press(&kit, KIT_COMPANION, SELECTED_CONFIRM_DOWN);
  assert(kit.companion.page == COMP_CARGO);
  press(&kit, KIT_COMPANION, SELECTED_CONFIRM_DOWN);
  assert(kit.companion.page == COMP_SEND_REVIEW);
  uint32_t review_cargo = lab.game.expedition_data;
  kit_tick(&kit, 170);
  assert(lab.game.expedition_data == review_cargo);
  assert(kit_link(&kit, KIT_COMPANION, 0));
  press(&kit, KIT_COMPANION, SELECTED_CONFIRM_DOWN);
  assert(kit.journal.phase == KIT_WAITING);
  uint32_t cargo = kit.journal.cargo[0];
  assert(cargo > 0);
  kit_tick(&kit, 200);
  assert(lab.game.expedition_data == cargo && lab.game.data == 0);
  /* Restart with an offline sealed haul retains intent and original identity.
   */
  SelectedLab reopened;
  selected_lab_init(&reopened);
  assert(selected_lab_load(&reopened, path, 300));
  DeviceKit recovered;
  assert(kit_init(&recovered, &reopened, 300));
  assert(recovered.journal.phase == KIT_WAITING &&
         !recovered.journal.companion_online);
  assert(kit_link(&recovered, KIT_COMPANION, 1));
  press(&recovered, KIT_LAB, SELECTED_DOWN_DOWN);
  press(&recovered, KIT_LAB, SELECTED_CONFIRM_DOWN);
  unsigned old_frame = kit_revision(&recovered, KIT_LAB);
  kit_input(&recovered, KIT_LAB, SELECTED_READY, old_frame);
  kit_input(&recovered, KIT_LAB, SELECTED_CONFIRM_DOWN, old_frame);
  kit_tick(&recovered, 302);
  kit_input(&recovered, KIT_LAB, SELECTED_CONFIRM_UP, old_frame);
  assert(reopened.game.data ==
         0); /* Arrival cannot authorize an old held Confirm. */
  assert(recovered.journal.phase == KIT_ARRIVED);
  assert(kit_link(&recovered, KIT_DOCK, 0));
  assert(kit_link(&recovered, KIT_COMPANION, 0));
  press(&recovered, KIT_LAB, SELECTED_CONFIRM_DOWN);
  assert(recovered.journal.phase == KIT_ACK_PENDING &&
         reopened.game.data == cargo);
  assert(reopened.game.sample_count == 1 &&
         recovered.journal.dock_stock[0] == 0);
  uint64_t sequence = reopened.game.last_operation_sequence;
  press(&recovered, KIT_LAB, SELECTED_CONFIRM_DOWN);
  kit_tick(&recovered, 310);
  assert(reopened.game.last_operation_sequence == sequence &&
         reopened.game.data == cargo);
  assert(recovered.journal.phase == KIT_ACK_PENDING);
  /* Crash after game commit but before receipt save: intent is reconciled, not
   * credited twice. */
  recovered.journal.phase = KIT_COMMITTING;
  persist_fixture(&recovered);
  selected_lab_init(&lab);
  assert(selected_lab_load(&lab, path, 400));
  assert(kit_init(&kit, &lab, 400));
  assert(kit.journal.phase == KIT_ACK_PENDING && lab.game.data == cargo &&
         lab.game.sample_count == 1);
  /* A missing/rolled-back accepted world cannot authorize an acknowledgement.
   */
  GameState accepted_world = lab.game;
  game_state_init(&lab.game);
  assert(game_state_save(path, &lab.game) == 0);
  SelectedLab missing;
  selected_lab_init(&missing);
  assert(selected_lab_load(&missing, path, 401));
  DeviceKit missing_kit;
  assert(!kit_init(&missing_kit, &missing, 401));
  assert(missing_kit.failed);
  lab.game = accepted_world;
  assert(game_state_save(path, &lab.game) == 0);
  assert(kit_link(&kit, KIT_COMPANION, 1));
  kit_tick(&kit, 402);
  assert(kit.journal.phase == KIT_COMPLETE && !lab.game.expedition_id[0]);
  assert(kit_link(&kit, KIT_DOCK, 1));
  kit_tick(&kit, 403);
  assert(kit.journal.dock_stock[0] == cargo && kit.journal.dock_samples == 1);
  /* No silent recovery from a lost required journal after accepted cargo
   * cleared. */
  assert(unlink(kit.journal_path) == 0);
  SelectedLab blocked;
  selected_lab_init(&blocked);
  assert(selected_lab_load(&blocked, path, 500));
  DeviceKit blocked_kit;
  assert(!kit_init(&blocked_kit, &blocked, 500));
  assert(blocked_kit.failed && blocked.storage_error &&
         blocked.game.data == cargo);
  char marker[580];
  snprintf(marker, sizeof(marker), "%s.required", kit.journal_path);
  unlink(marker);
  unlink(path);
  rmdir(directory);
  puts("Three-device ownership, offline handoff, duplicate, recovery and Dock "
       "cache checks passed");
  return 0;
}
