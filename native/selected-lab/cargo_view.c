#include "cargo_view.h"
#include "expedition_render.h"
#include <stdio.h>
#include <string.h>

int kit_cargo_projection(const DeviceKit *kit, CompanionCargoView *out) {
  if (!kit || !out || kit->companion.page != COMP_CARGO)
    return 0;
  memset(out, 0, sizeof(*out));
  const KitView *view = &kit->companion;
  const GameState *game = &kit->lab->game;
  out->phase = kit->journal.phase;
  out->failed = kit->failed;
  out->focus = view->focus;
  out->revision = view->revision;
  out->epoch = view->epoch;
  out->suspended = view->suspended;
  for (unsigned button = 0; button < 10; ++button) {
    out->held |= view->gestures[button].held;
    out->pressed |= view->gestures[button].held && view->gestures[button].allowed;
  }
  int sealed = out->phase >= KIT_WAITING && out->phase <= KIT_ACK_PENDING;
  ExpeditionFieldView field;
  if (kit_field_projection(kit, &field)) {
    memcpy(out->supplies, field.earned, sizeof(out->supplies));
    memcpy(out->delivered, field.sent, sizeof(out->delivered));
    out->capsules = field.capsule_count;
    out->capsule_capacity = field.capsule_capacity;
    out->accepted = field.delivery_accepted;
    out->delivered_capsules = field.sent_capsule_count;
    snprintf(out->identity, sizeof(out->identity), "%s", field.outing_id);
  } else {
    const uint32_t current[] = {game->expedition_data, game->expedition_energy,
                                game->expedition_essence};
    out->accepted = out->phase >= KIT_ACK_PENDING && !game->expedition_id[0];
    out->delivered_capsules = out->accepted && kit_received_sample(kit) ? 1 : 0;
    out->capsule_capacity = 1;
    for (unsigned resource = 0; resource < 3; ++resource) {
      out->delivered[resource] = kit->journal.cargo[resource] / GAME_SUPPLY_UNIT;
      out->supplies[resource] = out->accepted ? 0 :
          (sealed ? kit->journal.cargo[resource] : current[resource]) / GAME_SUPPLY_UNIT;
    }
    snprintf(out->identity, sizeof(out->identity), "%s",
             game->expedition_id[0] ? game->expedition_id : kit->journal.haul_id);
  }
  snprintf(out->title, sizeof(out->title), "%s", out->accepted ? "Cargo empty" : "Cargo");
  snprintf(out->context, sizeof(out->context), "%s",
           out->accepted ? "Expedition ended" : sealed ? kit_stage(kit) : kit_route(kit));
  snprintf(out->capsule, sizeof(out->capsule), "%s",
           out->capsules ? "1 sealed sample" : "No sample in cargo");
  if (out->accepted) {
    snprintf(out->detail, sizeof(out->detail),
             "Delivery record: %u Data / %u Energy / %u Essence. %s",
             out->delivered[0], out->delivered[1], out->delivered[2],
             out->delivered_capsules ? "1 sample delivered to Lab." : "Supplies stored at Lab.");
  } else {
    snprintf(out->detail, sizeof(out->detail), "%s",
             out->capsules ? "Contents unknown" : sealed ? "This expedition cannot resume." :
             "Whole items only / preparation stays here.");
  }
  unsigned total = out->supplies[0] + out->supplies[1] + out->supplies[2];
  snprintf(out->capacity, sizeof(out->capacity), "Supplies %u / %u   Capsules %u / %u",
           total, GAME_CARGO_CAPACITY / GAME_SUPPLY_UNIT, out->capsules, out->capsule_capacity);
  const char *feedback = view->message[0] ? view->message :
      out->accepted ? (out->phase == KIT_COMPLETE ? "Delivery complete / choose a new outing." : "Lab accepted / receipt pending.") :
      sealed ? kit_stage(kit) : kit->journal.companion_online ? "Lab link available" : "Lab offline";
  if (out->failed) {
    snprintf(out->detail, sizeof(out->detail), "%s", out->accepted ?
             "Cargo transferred. Delivery record needs recovery." :
             "Storage unavailable. Cargo preserved.");
    feedback = "Storage unavailable";
  }
  snprintf(out->feedback, sizeof(out->feedback), "%s", feedback);
  out->action_count = out->failed ? 0 : kit_option_count(kit, KIT_COMPANION);
  if (out->action_count > 2)
    return 0;
  for (unsigned action = 0; action < out->action_count; ++action)
    snprintf(out->actions[action], sizeof(out->actions[action]), "%s",
             kit_option(kit, KIT_COMPANION, action));
  return 1;
}
