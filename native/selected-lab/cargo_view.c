#include "cargo_view.h"
#include "expedition_render.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

int kit_cargo_facts(const DeviceKit *kit, CompanionCargoFacts *out) {
  if (!kit || !out || !kit->lab)
    return 0;
  memset(out, 0, sizeof(*out));
  const GameState *game = &kit->lab->game;
  unsigned phase = kit->journal.phase;
  int sealed = phase >= KIT_WAITING && phase <= KIT_ACK_PENDING;
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
    out->accepted = phase >= KIT_ACK_PENDING && !game->expedition_id[0];
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
  return 1;
}
int kit_cargo_projection(const DeviceKit *kit, CompanionCargoView *out) {
  if (!kit || !out || (kit->companion.page != COMP_CARGO &&
                       kit->companion.page != COMP_SEND_REVIEW))
    return 0;
  CompanionCargoFacts facts;
  if (!kit_cargo_facts(kit, &facts)) return 0;
  memset(out, 0, sizeof(*out));
  int review = kit->companion.page == COMP_SEND_REVIEW;
  out->screen = review ? COMPANION_SEND_SCREEN : COMPANION_CARGO_SCREEN;
  memcpy(out->supplies, facts.supplies, sizeof(out->supplies));
  memcpy(out->delivered, facts.delivered, sizeof(out->delivered));
  out->capsules = facts.capsules;
  out->capsule_capacity = facts.capsule_capacity;
  out->delivered_capsules = facts.delivered_capsules;
  out->accepted = facts.accepted;
  strcpy(out->identity, facts.identity);
  const KitView *view = &kit->companion;
  out->phase = kit->journal.phase;
  out->failed = kit->failed || kit->lab->storage_error;
  out->focus = view->focus;
  out->revision = view->revision;
  out->epoch = view->epoch;
  out->suspended = view->suspended;
  for (unsigned button = 0; button < 10; ++button) {
    out->held |= view->gestures[button].held;
    out->pressed |= view->gestures[button].held && view->gestures[button].allowed;
  }
  int sealed = out->phase >= KIT_WAITING && out->phase <= KIT_ACK_PENDING;
  snprintf(out->title, sizeof(out->title), "%s", out->accepted ? "Cargo empty" :
           sealed ? "Cargo sealed" : review ? "Return to Lab" : "Cargo");
  snprintf(out->context, sizeof(out->context), "%s",
           out->accepted ? "Expedition ended" : sealed ? kit_stage(kit) : kit_route(kit));
  snprintf(out->capsule, sizeof(out->capsule), "%s",
           out->capsules ? "1 sealed sample" : "No sample in cargo");
  if (out->accepted) {
    snprintf(out->detail, sizeof(out->detail),
             "Delivery record: %" PRIu32 " Data / %" PRIu32 " Energy / %" PRIu32 " Essence. %s",
             out->delivered[0], out->delivered[1], out->delivered[2],
             out->delivered_capsules ? "1 sample delivered to Lab." : "Supplies stored at Lab.");
  } else {
    const char *detail = sealed ? "This expedition cannot resume." :
        review ? (out->capsules ? "Contents unknown\nSeals cargo; exploration stops."
                               : "Seals cargo; exploration stops.") :
        out->capsules ? "Contents unknown" : "Whole items / source progress retained.";
    snprintf(out->detail, sizeof(out->detail), "%s", detail);
  }
  /* Legacy timed outings record their completed sample on acceptance. Show
   * that expected result separately; it is not an already carried capsule. */
  const GameState *game = &kit->lab->game;
  if (review && !sealed && !out->accepted && !game->field.version &&
      game->expedition_id[0] && game->expedition_elapsed >= GAME_EXPEDITION_SECONDS) {
    if (game->sample_count < GAME_MAX_SAMPLES) {
      snprintf(out->capsule, sizeof(out->capsule), "%s", "Sample ready at Lab");
      snprintf(out->detail, sizeof(out->detail), "%s",
               "Recorded on acceptance.\nSeals cargo; exploration stops.");
    } else {
      snprintf(out->capsule, sizeof(out->capsule), "%s", "No sample / Lab shelf full");
    }
  }
  uint32_t total = out->supplies[0] + out->supplies[1] + out->supplies[2];
  snprintf(out->capacity, sizeof(out->capacity), "Supplies %" PRIu32 " / %u   Capsules %" PRIu32 " / %" PRIu32,
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
  /* Normal sealing returns to Cargo. A restored/defensive sealed review must
   * never offer Keep as cancellation or Send as a second seal operation. */
  out->action_count = out->failed || (review && (sealed || out->accepted))
                         ? 0 : kit_option_count(kit, KIT_COMPANION);
  if (out->action_count > 2)
    return 0;
  for (unsigned action = 0; action < out->action_count; ++action)
    snprintf(out->actions[action], sizeof(out->actions[action]), "%s",
             kit_option(kit, KIT_COMPANION, action));
  snprintf(out->footer, sizeof(out->footer), "%s", out->failed ? "Storage recovery required" :
           review && !sealed && !out->accepted ? "Back: Keep cargo" :
           "Up/Down: choose / Back: modes");
  return 1;
}
