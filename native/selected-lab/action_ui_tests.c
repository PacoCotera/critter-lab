#define _POSIX_C_SOURCE 200809L
#include "action_view.h"
#include "home_view.h"
#include "reception_view.h"
#include "research_view.h"
#include "native_ui.h"
#include "core_art.h"
#include "../ui/display.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Presentation fixtures are synthetic accepted knowledge/individuals, not
 * played acquisitions or births. The physical journey uses real save commits. */
static void fixture(SelectedLab *lab, DeviceKit *kit) {
  selected_lab_init(lab);
  memset(kit, 0, sizeof(*kit));
  kit->lab = lab;
  kit->journal.companion_online = kit->journal.dock_online = 1;
  lab->kit_mode = 1;
  lab->page = V1_CREATE;
  lab->game.data = lab->game.energy = lab->game.essence = 2000;
  lab->game.sample_count = 2;
  for (unsigned index = 0; index < 2; ++index) {
    snprintf(lab->game.samples[index].id, sizeof(lab->game.samples[index].id),
        "fixture-action-%u", index);
    snprintf(lab->game.samples[index].origin_expedition_id,
        sizeof(lab->game.samples[index].origin_expedition_id), "fixture-origin-%u", index);
    lab->game.samples[index].supported_candidates = PIP_SAMPLE_CANDIDATE_MASK;
    pip_pin_sample_profile(&lab->game, index);
    assert(pip_sample_metadata_valid(&lab->game, index));
  }
}

static void complete(SelectedLab *lab) {
  for (unsigned method = 0; method < PIP_DISCOVERY_METHOD_COUNT; ++method)
    assert(pip_record_investigation(&lab->game, 0, method));
  assert(pip_record_investigation(&lab->game, 1, 0));
  assert(pip_record_investigation(&lab->game, 1, 2));
}

static void draft(SelectedLab *lab, unsigned preference) {
  PipSupportedCandidate candidate;
  assert(selected_lab_candidate(lab, lab->sample, preference, &candidate));
  SelectedCreationDraft *review = &lab->creation_draft;
  review->valid = 1;
  review->sample = lab->sample;
  review->preference = preference;
  strcpy(review->sample_id, lab->game.samples[lab->sample].id);
  strcpy(review->content_version, pip_sample_content_version(&lab->game, lab->sample));
  strcpy(review->candidate_id, candidate.id);
  lab->creation_preference = preference;
}

static void resident_fixture(SelectedLab *lab, unsigned count) {
  lab->game.individual_count = count;
  for (unsigned index = 0; index < count; ++index) {
    PipSupportedCandidate candidate;
    assert(selected_lab_candidate(lab, index % 2, index % 2, &candidate));
    GameIndividual *individual = &lab->game.individuals[index];
    memset(individual, 0, sizeof(*individual));
    snprintf(individual->id, sizeof(individual->id), "fixture-resident-%u", index);
    strcpy(individual->source_sample_id, lab->game.samples[index % 2].id);
    strcpy(individual->origin_kind, "parentless-founder");
    individual->genome = candidate.genome;
    individual->expression = candidate.expression;
    individual->origin_founder = individual->revealed = individual->habitat = 1;
    strcpy(individual->art_id, pip_content_art_id(&candidate.genome));
    strcpy(individual->art_version, PIP_ART_VERSION);
    pip_pin_individual_art(&lab->game, index, candidate.id);
  }
  lab->resident = 0;
}

static void button(SelectedLab *lab, SelectedInput down) {
  unsigned revision = lab->revision;
  selected_lab_input(lab, SELECTED_READY, 0, revision);
  selected_lab_input(lab, down, 0, revision);
  selected_lab_input(lab, (SelectedInput)(down + 1), 0, revision);
}

static void equal_bmps(FILE *first, FILE *second) {
  assert(ftell(first) == 1843254 && ftell(second) == 1843254);
  rewind(first);
  rewind(second);
  uint8_t left[4096], right[4096];
  size_t count;
  while ((count = fread(left, 1, sizeof(left), first)) != 0)
    assert(fread(right, 1, count, second) == count && !memcmp(left, right, count));
  assert(!ferror(first) && !ferror(second) && fgetc(second) == EOF);
}

static void export_fixture(DeviceKit *kit, NativeUiContext *context, const char *name) {
  const char *directory = getenv("BEECHO_ACTION_PROOF");
  if (!directory) return;
  char path[768];
  assert(snprintf(path, sizeof(path), "%s/fixture-action-%s.bmp", directory, name) < (int)sizeof(path));
  FILE *output = fopen(path, "wb");
  assert(output && kit_bmp_ui(kit, KIT_LAB, output, context, 1) && !fclose(output));
}

static void assert_routes(DeviceKit *kit, NativeUiContext *context, const char *name) {
  SelectedLab before = *kit->lab;
  DeviceKit kit_before = *kit;
  FILE *standalone = tmpfile(), *generic = tmpfile(), *persistent = tmpfile();
  assert(standalone && generic && persistent);
  assert(selected_lab_bmp(kit->lab, standalone));
  assert(kit_bmp(kit, KIT_LAB, generic));
  assert(kit_bmp_ui(kit, KIT_LAB, persistent, context, 1));
  assert(!fseek(persistent, 54, SEEK_SET));
  uint8_t background[3], pixel[3];
  assert(fread(background, 1, 3, persistent) == 3);
  unsigned painted = 0;
  while (fread(pixel, 1, 3, persistent) == 3) painted += memcmp(pixel, background, 3) != 0;
  assert(painted > 2000 && !ferror(persistent));
  assert(!fseek(persistent, 0, SEEK_END));
  equal_bmps(standalone, persistent);
  equal_bmps(generic, persistent);
  assert(!fclose(standalone) && !fclose(generic) && !fclose(persistent));
  export_fixture(kit, context, name);
  assert(!memcmp(&before, kit->lab, sizeof(before)) && !memcmp(&kit_before, kit, sizeof(*kit)));
}

static void assert_failed_routes(DeviceKit *kit, NativeUiContext *context) {
  FILE *output = tmpfile();
  assert(output && !selected_lab_bmp(kit->lab, output) && !ftell(output));
  assert(!kit_bmp(kit, KIT_LAB, output) && !ftell(output));
  assert(!kit_bmp_ui(kit, KIT_LAB, output, context, 1) && !ftell(output));
  assert(!fclose(output));
}

static void projection_cases(void) {
  SelectedLab lab;
  DeviceKit kit;
  LabActionView view;
  fixture(&lab, &kit);
  SelectedLab before = lab;
  assert(selected_lab_action_projection(&lab, 0, &view));
  assert(view.page == LAB_ACTION_CREATE && view.detail == LAB_ACTION_CREATE_LOCKED);
  assert(view.option_count == 1 && !view.candidate_authorized && view.art == LAB_ACTION_ART_TOOLS);
  assert(!memcmp(&lab, &before, sizeof(lab)));
  assert(pip_record_investigation(&lab.game, 0, 0));
  assert(selected_lab_action_projection(&lab, 0, &view) && !view.candidate_authorized);
  assert(view.art != LAB_ACTION_ART_PLAIN && view.art != LAB_ACTION_ART_MARKED);
  assert(pip_record_investigation(&lab.game, 0, 1));
  assert(pip_record_investigation(&lab.game, 0, 2));
  for (unsigned preference = 0; preference < 2; ++preference) {
    lab.focus = preference;
    assert(selected_lab_action_projection(&lab, 0, &view));
    assert(view.detail == LAB_ACTION_CREATE_AVAILABLE && view.candidate_authorized);
    assert(view.art == (preference ? LAB_ACTION_ART_MARKED : LAB_ACTION_ART_PLAIN));
    assert(!strcmp(view.reference_id, preference ? "A1" : "A0"));
    assert(view.costs[0] == 5 && view.costs[1] == 5 && view.costs[2] == 5 && view.stock[0] == 20);
  }
  draft(&lab, 1);
  lab.page = V1_CREATE_REVIEW;
  lab.focus = 0;
  assert(selected_lab_action_projection(&lab, 0, &view) && view.draft_valid);
  assert(view.detail == LAB_ACTION_REVIEW_VALID && view.art == LAB_ACTION_ART_MARKED);
  strcpy(lab.creation_draft.sample_id, "different-sample");
  assert(selected_lab_action_projection(&lab, 0, &view) && !view.draft_valid && !view.candidate_authorized);
  assert(view.detail == LAB_ACTION_REVIEW_STALE && view.art != LAB_ACTION_ART_MARKED);
  fixture(&lab, &kit);
  complete(&lab);
  lab.sample = 1;
  for (unsigned preference = 0; preference < 2; ++preference) {
    lab.focus = preference;
    assert(selected_lab_action_projection(&lab, 0, &view));
    assert(view.candidate_authorized && view.art == LAB_ACTION_ART_PLAIN);
    assert(strstr(view.form_title, preference ? "Burst" : "Steady"));
  }
  resident_fixture(&lab, 1);
  lab.page = V1_HABITAT;
  lab.focus = 0;
  assert(selected_lab_action_projection(&lab, 0, &view));
  assert(view.resident_visible && !strcmp(view.resident_id, lab.game.individuals[0].id));
  assert(view.art == LAB_ACTION_ART_PLAIN && !strcmp(view.source_sample_id, lab.game.individuals[0].source_sample_id));
  /* The saved original is authoritative even if copied coat text differs. */
  lab.game.individuals[0].expression.pale_markings = 1;
  assert(selected_lab_action_projection(&lab, 0, &view) && view.art == LAB_ACTION_ART_PLAIN);
  lab.game.individuals[0].expression.pale_markings = 0;
  GameSampleMetadata knowledge = lab.game.sample_metadata[0];
  memset(&lab.game.sample_metadata[0], 0, sizeof(knowledge));
  lab.game.samples[0].decoded_facts = lab.game.samples[0].decoded_studies = 0;
  assert(selected_lab_action_projection(&lab, 0, &view));
  assert(view.art == LAB_ACTION_ART_PLAIN && strstr(view.form_title, "Plain"));
  lab.game.sample_metadata[0] = knowledge;
  lab.game.individual_metadata[0].original_art_sha256[0] = 'x';
  assert(selected_lab_action_projection(&lab, 0, &view) && view.art == LAB_ACTION_ART_PENDING);
  lab.game.individuals[0].revealed = 0;
  assert(selected_lab_action_projection(&lab, 0, &view) && !view.resident_visible);
  assert(view.art != LAB_ACTION_ART_PLAIN && view.art != LAB_ACTION_ART_MARKED);
}

static void assert_native_portrait(const uint8_t *frame, CoreArtId id) {
  const CoreArtSprite *source = core_art_sprite(id);
  assert(source && source->width == 261 && source->height == 289);
  unsigned checked = 0;
  for (unsigned row = 0; row < source->height; ++row)
    for (unsigned column = 0; column < source->width; ++column) {
      const uint8_t *pixel = source->rgba + (row * source->width + column) * 4;
      if (pixel[3] == 255) {
        const uint8_t *actual = frame + ((247 + row) * SELECTED_LAB_WIDTH + 405 + column) * 3;
        if (memcmp(actual, pixel, 3)) {
          fprintf(stderr, "Original portrait mismatch art=%u source=%u,%u frame=%u,%u actual=%u,%u,%u expected=%u,%u,%u\n",
              (unsigned)id, column, row, 405 + column, 247 + row,
              actual[0], actual[1], actual[2], pixel[0], pixel[1], pixel[2]);
          fflush(stderr);
        }
        assert(!memcmp(actual, pixel, 3));
        ++checked;
      }
    }
  assert(checked > 1000);
}

static void malformed(NativeUiContext *context) {
  SelectedLab lab;
  DeviceKit kit;
  LabActionView view, invalid;
  fixture(&lab, &kit);
  complete(&lab);
  assert(selected_lab_action_projection(&lab, 0, &view));
  invalid = view;
  invalid.page = (LabActionPage)-1;
  assert(!native_ui_actions(context, &invalid));
  invalid = view;
  invalid.option_count = LAB_ACTION_OPTIONS + 1;
  assert(!native_ui_actions(context, &invalid));
  invalid = view;
  invalid.option_count = LAB_ACTION_OPTIONS;
  assert(!native_ui_actions(context, &invalid));
  invalid = view;
  invalid.costs[0] = 4;
  assert(!native_ui_actions(context, &invalid));
  invalid = view;
  invalid.focus = invalid.option_count;
  assert(!native_ui_actions(context, &invalid));
  invalid = view;
  memset(invalid.form_title, 'x', sizeof(invalid.form_title));
  assert(!native_ui_actions(context, &invalid));
  invalid = view;
  invalid.candidate_authorized = 0;
  assert(!native_ui_actions(context, &invalid));
  invalid = view;
  invalid.resident_visible = 1;
  assert(!native_ui_actions(context, &invalid));
  invalid = view;
  invalid.page = LAB_ACTION_INCUBATION;
  invalid.detail = LAB_ACTION_INCUBATION_READY;
  assert(!native_ui_actions(context, &invalid));
  lab.game.sample_count = GAME_MAX_SAMPLES + 1;
  memset(&invalid, 0xa5, sizeof(invalid));
  view = invalid;
  assert(!selected_lab_action_projection(&lab, 0, &view) && !memcmp(&view, &invalid, sizeof(view)));
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  lab.game.individual_count = GAME_MAX_INDIVIDUALS + 1;
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  memset(lab.game.samples[0].id, 'x', sizeof(lab.game.samples[0].id));
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  lab.sample = GAME_MAX_SAMPLES;
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  complete(&lab);
  draft(&lab, 0);
  lab.page = V1_CREATE_REVIEW;
  memset(lab.creation_draft.candidate_id, 'x', sizeof(lab.creation_draft.candidate_id));
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  complete(&lab);
  resident_fixture(&lab, 1);
  lab.page = V1_REVEAL;
  memset(lab.game.individual_metadata[0].original_art_sha256, 'x',
      sizeof(lab.game.individual_metadata[0].original_art_sha256));
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  lab.page = V1_INCUBATION;
  lab.game.incubation_active = 1;
  lab.game.incubation_sample = GAME_MAX_SAMPLES;
  assert_failed_routes(&kit, context);
}

static void routes_exports(void) {
  SelectedLab lab;
  DeviceKit kit;
  LabActionView view;
  NativeUiContext *context = native_ui_create_device(KIT_LAB);
  assert(context);
  fixture(&lab, &kit);
  assert_routes(&kit, context, "locked");
  assert(pip_record_investigation(&lab.game, 0, 0));
  assert_routes(&kit, context, "partial-p");
  assert(pip_record_investigation(&lab.game, 0, 1));
  assert(pip_record_investigation(&lab.game, 0, 2));
  for (unsigned preference = 0; preference < 2; ++preference) {
    lab.focus = preference;
    assert(selected_lab_action_projection(&lab, 0, &view));
    const uint8_t *frame = native_ui_actions(context, &view);
    assert(frame);
    export_fixture(&kit, context, preference ? "A1-before-pixels" : "A0-before-pixels");
    assert_native_portrait(frame, preference ? CORE_ART_PIP_MARKED : CORE_ART_PIP_PLAIN);
    if (!preference) {
      uint8_t *expected = malloc(SELECTED_LAB_WIDTH * SELECTED_LAB_HEIGHT * 3);
      assert(expected);
      memcpy(expected, frame, SELECTED_LAB_WIDTH * SELECTED_LAB_HEIGHT * 3);
      memset(&view, 0, sizeof(view));
      lv_display_t *display = lv_display_get_default();
      assert(display && ui_display_count() == 1);
      lv_obj_invalidate(lv_display_get_screen_active(display));
      lv_refr_now(display);
      assert(!memcmp(expected, frame, SELECTED_LAB_WIDTH * SELECTED_LAB_HEIGHT * 3));
      free(expected);
    }
    assert_routes(&kit, context, preference ? "A1" : "A0");
  }
  draft(&lab, 1);
  lab.page = V1_CREATE_REVIEW;
  lab.focus = 0;
  assert_routes(&kit, context, "review");
  lab.game.data = 0;
  lab.game.energy = 100;
  lab.game.essence = 400;
  assert_routes(&kit, context, "shortage");
  strcpy(lab.creation_draft.candidate_id, "unknown");
  assert_routes(&kit, context, "stale-review");
  fixture(&lab, &kit);
  complete(&lab);
  lab.sample = 1;
  for (unsigned preference = 0; preference < 2; ++preference) {
    lab.focus = preference;
    assert_routes(&kit, context, preference ? "B1" : "B0");
  }
  fixture(&lab, &kit);
  lab.page = V1_INCUBATION;
  assert_routes(&kit, context, "incubator-empty");
  complete(&lab);
  resident_fixture(&lab, 1);
  lab.game.individuals[0].revealed = 0;
  lab.game.incubation_active = 1;
  lab.game.incubation_sample = lab.game.incubation_individual = 0;
  lab.game.incubation_elapsed = 7;
  assert(selected_lab_action_projection(&lab, 0, &view));
  assert(view.art == LAB_ACTION_ART_INCUBATOR_ACTIVE && !view.resident_visible);
  assert(view.elapsed_ms == 7000 && view.duration_ms == GAME_INCUBATION_SECONDS * 1000);
  assert_routes(&kit, context, "incubator-active");
  lab.game.incubation_ready = 1;
  lab.game.incubation_elapsed = GAME_INCUBATION_SECONDS;
  assert(selected_lab_action_projection(&lab, 0, &view));
  assert(view.art == LAB_ACTION_ART_INCUBATOR_READY && !view.resident_visible && !view.candidate_authorized);
  assert_routes(&kit, context, "incubator-ready");
  lab.page = V1_REVEAL;
  lab.game.incubation_active = lab.game.incubation_ready = 0;
  lab.game.incubation_sample = lab.game.incubation_individual = 255;
  lab.game.individuals[0].revealed = 1;
  assert_routes(&kit, context, "reveal");
  lab.game.individuals[0].art_pending = 1;
  assert_routes(&kit, context, "pending");
  lab.game.individuals[0].art_pending = 0;
  lab.page = V1_HABITAT;
  lab.game.individuals[0].care_visits = GAME_MAX_CARE_VISITS;
  assert_routes(&kit, context, "habitat");
  lab.game.individuals[0].revealed = 0;
  assert_routes(&kit, context, "habitat-empty");
  lab.page = V1_CRITTERS;
  assert_routes(&kit, context, "residents-empty");
  resident_fixture(&lab, GAME_MAX_INDIVIDUALS);
  lab.focus = 7;
  lab.resident = 7;
  for (unsigned index = 0; index < GAME_MAX_INDIVIDUALS; ++index) {
    memset(lab.game.individuals[index].id, 'R', 38);
    lab.game.individuals[index].id[38] = (char)('0' + index);
    lab.game.individuals[index].id[39] = 0;
  }
  memset(lab.game.individuals[7].source_sample_id, 'S', 39);
  lab.game.individuals[7].source_sample_id[39] = 0;
  lab.game.data = lab.game.energy = lab.game.essence = 1000000;
  assert_routes(&kit, context, "residents-max");
  lab.storage_error = 1;
  memset(lab.message, 'M', 95);
  lab.message[95] = 0;
  assert_routes(&kit, context, "storage-error");
  malformed(context);
  native_ui_destroy(context);
}

static void physical_journey(void) {
  char directory[] = "/tmp/bee-actions-XXXXXX";
  assert(mkdtemp(directory));
  char path[512];
  snprintf(path, sizeof(path), "%s/state", directory);
  SelectedLab lab;
  selected_lab_init(&lab);
  assert(selected_lab_load(&lab, path, 100));
  lab.game.data = lab.game.energy = lab.game.essence = 2000;
  strcpy(lab.game.expedition_id, "action-controls-intake");
  lab.game.expedition_elapsed = GAME_EXPEDITION_SECONDS;
  GameCommand unload = {0};
  unload.type = GAME_COMMAND_EXPEDITION_UNLOAD;
  unload.operation_id = "action-controls-intake";
  unload.sequence = lab.game.last_operation_sequence + 1;
  assert(game_apply(path, &lab.game, &unload) == GAME_OK);
  assert(lab.game.sample_count == 1);
  for (unsigned method = 0; method < PIP_DISCOVERY_METHOD_COUNT; ++method)
    assert(pip_record_investigation(&lab.game, 0, method));
  assert(game_state_save(path, &lab.game) == 0);
  lab.page = V1_CREATE;
  lab.focus = 1;
  GameState before = lab.game;
  button(&lab, SELECTED_RIGHT_DOWN);
  assert(lab.page == V1_CREATE && !memcmp(&before, &lab.game, sizeof(before)));
  button(&lab, SELECTED_CONFIRM_DOWN);
  assert(lab.page == V1_CREATE_REVIEW && lab.creation_draft.valid && lab.creation_preference == 1);
  assert(!memcmp(&before, &lab.game, sizeof(before)));
  NativeUiContext *context = native_ui_create_device(KIT_LAB);
  LabActionView view;
  assert(context && selected_lab_action_projection(&lab, 0, &view) && native_ui_actions(context, &view));
  assert(!memcmp(&before, &lab.game, sizeof(before)));
  button(&lab, SELECTED_BACK_DOWN);
  assert(lab.page == V1_CREATE && lab.focus == 1 && !memcmp(&before, &lab.game, sizeof(before)));
  button(&lab, SELECTED_CONFIRM_DOWN);
  unsigned review_revision = lab.revision;
  selected_lab_input(&lab, SELECTED_CONFIRM_UP, 0, review_revision - 1);
  assert(!memcmp(&before, &lab.game, sizeof(before)));
  button(&lab, SELECTED_CONFIRM_DOWN);
  assert(lab.page == V1_INCUBATION && lab.game.individual_count == 1 && lab.game.samples[0].incubated);
  assert(lab.game.data == before.data - 500 && lab.game.energy == before.energy - 500 && lab.game.essence == before.essence - 500);
  assert(lab.game.last_operation_sequence == before.last_operation_sequence + 1);
  assert(!lab.game.individuals[0].revealed);
  GameState created = lab.game;
  selected_lab_input(&lab, SELECTED_CONFIRM_UP, 0, review_revision);
  assert(!memcmp(&created, &lab.game, sizeof(created)));
  assert(selected_lab_action_projection(&lab, 0, &view) && view.art == LAB_ACTION_ART_INCUBATOR_ACTIVE);
  assert(native_ui_actions(context, &view) && !memcmp(&created, &lab.game, sizeof(created)));
  selected_lab_tick_devices(&lab, lab.clock + GAME_INCUBATION_SECONDS, 0, 1);
  assert(lab.game.incubation_ready && !lab.game.individuals[0].revealed);
  assert(selected_lab_action_projection(&lab, 0, &view) && view.art == LAB_ACTION_ART_INCUBATOR_READY);
  GameState ready = lab.game;
  assert(native_ui_actions(context, &view) && !memcmp(&ready, &lab.game, sizeof(ready)));
  button(&lab, SELECTED_RIGHT_DOWN);
  assert(!memcmp(&ready, &lab.game, sizeof(ready)));
  button(&lab, SELECTED_CONFIRM_DOWN);
  assert(lab.page == V1_REVEAL && lab.game.individuals[0].revealed && !lab.game.incubation_active);
  GameIndividual identity = lab.game.individuals[0];
  GameState opened = lab.game;
  assert(selected_lab_action_projection(&lab, 0, &view) && native_ui_actions(context, &view));
  assert(!memcmp(&opened, &lab.game, sizeof(opened)));
  button(&lab, SELECTED_CONFIRM_DOWN);
  assert(lab.page == V1_HABITAT && lab.game.individuals[0].habitat == 1 && !lab.game.individuals[0].care_visits);
  GameState met = lab.game;
  button(&lab, SELECTED_RIGHT_DOWN);
  assert(!memcmp(&met, &lab.game, sizeof(met)));
  button(&lab, SELECTED_CONFIRM_DOWN);
  assert(lab.game.individuals[0].care_visits == 1);
  button(&lab, SELECTED_HOME_DOWN);
  assert(lab.page == V1_HOME);
  SelectedLab reloaded;
  selected_lab_init(&reloaded);
  assert(selected_lab_load(&reloaded, path, lab.clock));
  assert(reloaded.game.individual_count == 1 && reloaded.game.individuals[0].care_visits == 1);
  assert(!strcmp(identity.id, reloaded.game.individuals[0].id));
  assert(!strcmp(identity.source_sample_id, reloaded.game.individuals[0].source_sample_id));
  assert(!strcmp(identity.art_id, reloaded.game.individuals[0].art_id));
  assert(!memcmp(&identity.genome, &reloaded.game.individuals[0].genome, sizeof(identity.genome)));
  native_ui_destroy(context);
  unlink(path);
  char lock[520];
  snprintf(lock, sizeof(lock), "%s.lock", path);
  unlink(lock);
  rmdir(directory);
}

static void three_context_memory(void) {
  SelectedLab lab;
  DeviceKit kit;
  fixture(&lab, &kit);
  complete(&lab);
  resident_fixture(&lab, GAME_MAX_INDIVIDUALS);
  NativeUiContext *companion = native_ui_create(), *dock = native_ui_create_device(KIT_DOCK);
  NativeUiContext *context = native_ui_create_device(KIT_LAB);
  assert(companion && dock && context && ui_display_count() == 3);
  CompanionResidentView resident = {0};
  resident.portrait = RESIDENT_PORTRAIT_PLAIN;
  resident.count = resident.current = resident.online = 1;
  strcpy(resident.identity, "fixture-action-peer");
  DockView dock_view;
  assert(kit_dock_projection(&kit, &dock_view));
  LabHomeView home;
  lab.page = V1_HOME;
  assert(selected_lab_home_view(&lab, NULL, 0, &home));
  LabReceptionView reception;
  lab.page = V1_EXPEDITION;
  assert(kit_reception_projection(&kit, &reception));
  LabResearchView research;
  lab.page = V1_FINDING;
  lab.study = 2;
  assert(selected_lab_research_projection(&lab, 0, &research));
  LabActionView actions[6];
  const SelectedPage pages[] = {V1_CREATE, V1_CREATE_REVIEW, V1_INCUBATION, V1_REVEAL, V1_HABITAT, V1_CRITTERS};
  draft(&lab, 1);
  for (unsigned index = 0; index < 6; ++index) {
    lab.page = pages[index];
    lab.focus = index == 5 ? 7 : 0;
    assert(selected_lab_action_projection(&lab, 0, &actions[index]));
  }
  lv_mem_monitor_t first, final;
  for (unsigned cycle = 0; cycle < 200; ++cycle) {
    assert(native_ui_home(context, &home) && native_ui_reception(context, &reception));
    assert(native_ui_research(context, &research));
    for (unsigned index = 0; index < 6; ++index) assert(native_ui_actions(context, &actions[index]));
    assert(native_ui_resident(companion, &resident) && native_ui_dock(dock, &dock_view));
    if (cycle == 99) lv_mem_monitor(&first);
  }
  lv_mem_monitor(&final);
  printf("Actions/research/reception/Home plus Companion resident/Dock: used=%zu peak=%zu total=%zu first_free=%zu final_free=%zu\n",
      final.total_size - final.free_size, final.max_used, final.total_size, first.free_size, final.free_size);
  fflush(stdout);
  assert(first.free_size == final.free_size);
  native_ui_destroy(context);
  assert(ui_display_count() == 2 && native_ui_resident(companion, &resident) && native_ui_dock(dock, &dock_view));
  context = native_ui_create_device(KIT_LAB);
  assert(context && native_ui_home(context, &home) && native_ui_reception(context, &reception));
  assert(native_ui_research(context, &research) && native_ui_actions(context, &actions[5]));
  assert(native_ui_resident(companion, &resident) && native_ui_dock(dock, &dock_view));
  native_ui_destroy(context);
  native_ui_destroy(dock);
  native_ui_destroy(companion);
  assert(ui_display_count() == 0);
}

int main(void) {
  projection_cases();
  routes_exports();
  physical_journey();
  three_context_memory();
  puts("Lab action disclosure, native routes, physical authority and retained lifetime checks passed");
  return 0;
}
