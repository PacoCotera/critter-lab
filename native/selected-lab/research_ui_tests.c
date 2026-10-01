#define _POSIX_C_SOURCE 200809L
#include "research_view.h"
#include "home_view.h"
#include "reception_view.h"
#include "native_ui.h"
#include "core_art.h"
#include "../ui/display.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Copied research/art fixtures are synthetic retained samples, not played
 * acquisitions or births. The control test below uses real game commits. */
static void fixture(SelectedLab *lab, DeviceKit *kit) {
  selected_lab_init(lab);
  memset(kit, 0, sizeof(*kit));
  kit->lab = lab;
  lab->kit_mode = 1;
  lab->page = V1_SAMPLES;
  lab->game.data = lab->game.energy = lab->game.essence = 2000;
  lab->game.sample_count = 3;
  for (unsigned index = 0; index < 3; ++index) {
    GameSample *sample = &lab->game.samples[index];
    snprintf(sample->id, sizeof(sample->id), "fixture-research-%u", index);
    snprintf(sample->origin_expedition_id, sizeof(sample->origin_expedition_id),
             "fixture-origin-%u", index);
    sample->supported_candidates = PIP_SAMPLE_CANDIDATE_MASK;
    if (index < 2) pip_pin_sample_profile(&lab->game, index);
    assert(pip_sample_metadata_valid(&lab->game, index));
  }
  kit->journal.companion_online = kit->journal.dock_online = 1;
}

static void complete_a(SelectedLab *lab) {
  for (unsigned method = 0; method < PIP_DISCOVERY_METHOD_COUNT; ++method)
    assert(pip_record_investigation(&lab->game, 0, method));
}

static void complete_b(SelectedLab *lab) {
  assert(pip_record_investigation(&lab->game, 1, 0));
  assert(pip_record_investigation(&lab->game, 1, 2));
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
  const char *directory = getenv("BEECHO_RESEARCH_PROOF");
  if (!directory) return;
  char path[768];
  assert(snprintf(path, sizeof(path), "%s/fixture-research-%s.bmp", directory, name) < (int)sizeof(path));
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
  rewind(persistent);
  assert(!fseek(persistent, 54, SEEK_SET));
  uint8_t background[3], pixel[3];
  assert(fread(background, 1, 3, persistent) == 3);
  unsigned painted = 0;
  while (fread(pixel, 1, 3, persistent) == 3)
    painted += memcmp(pixel, background, 3) != 0;
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
  fixture(&lab, &kit);
  LabResearchView view;
  SelectedLab before = lab;
  assert(selected_lab_research_projection(&lab, 0, &view));
  assert(view.page == LAB_RESEARCH_SAMPLES && view.detail == LAB_RESEARCH_COLLECTION);
  assert(view.option_count == 4 && view.sample_count == 3 && view.awaiting == 3);
  assert(view.stock[0] == 20 && !memcmp(&before, &lab, sizeof(lab)));
  lab.page = V1_STUDIES;
  assert(selected_lab_research_projection(&lab, 0, &view));
  assert(view.topic_count == 3 && view.option_count == 4 && !view.known_method && view.useful);
  assert(view.costs[0] == 4 && !view.finding[0]);
  assert(!view.portraits[0] && !view.portraits[1]);
  assert(pip_record_investigation(&lab.game, 0, 0));
  lab.page = V1_FINDING;
  lab.study = 0;
  assert(selected_lab_research_projection(&lab, 0, &view));
  assert(view.partial_p && !view.complete && view.known_method && view.finding[0]);
  assert(view.art != LAB_RESEARCH_ART_PAIR && !view.portraits[0] && !view.portraits[1]);
  assert(strstr(view.missing, "Coat") || strstr(view.next, "Movement"));
  assert(pip_record_investigation(&lab.game, 0, 1));
  assert(pip_record_investigation(&lab.game, 0, 2));
  lab.study = 2;
  assert(selected_lab_research_projection(&lab, 0, &view));
  assert(view.complete && !view.partial_p && view.art == LAB_RESEARCH_ART_PAIR);
  assert(view.portraits[0] == LAB_RESEARCH_PORTRAIT_PLAIN && view.portraits[1] == LAB_RESEARCH_PORTRAIT_MARKED);
  assert(view.portrait_caption[0][0] && view.portrait_caption[1][0]);
  assert(!view.costs[0] && !view.costs[1] && !view.costs[2]);
  lab.game.samples[0].incubated = 1;
  assert(selected_lab_research_projection(&lab, 0, &view) && view.used && view.finding[0]);
  lab.sample = 1;
  lab.study = 1;
  assert(pip_record_investigation(&lab.game, 1, 2));
  assert(selected_lab_research_projection(&lab, 0, &view));
  assert(view.known_method && !view.useful && !view.complete && !view.partial_p);
  assert(!view.costs[0] && !view.costs[1] && !view.costs[2]);
  assert(strstr(view.finding, "paired effort") && !view.portraits[0] && !view.portraits[1]);
  assert(pip_record_investigation(&lab.game, 1, 0));
  lab.study = 2;
  assert(selected_lab_research_projection(&lab, 0, &view));
  assert(view.complete && view.art != LAB_RESEARCH_ART_PAIR);
  assert(strstr(view.finding, "Steady") && strstr(view.finding, "Burst-capable"));
  assert(!view.portraits[0] && !view.portraits[1]);
  lab.sample = 2;
  lab.study = 0;
  lab.game.samples[2].decoded_studies = lab.game.samples[2].decoded_facts = 1;
  assert(selected_lab_research_projection(&lab, 0, &view));
  assert(view.legacy && view.topic_count == 5 && view.known_method && view.art == LAB_RESEARCH_ART_CROWN);
  lab.page = V1_LIBRARY;
  lab.focus = 0;
  assert(selected_lab_research_projection(&lab, 0, &view));
  assert(view.option_count == 7); /* Three A, three B (movement resolved), one legacy. */
  lab.page = V1_HOME;
  assert(!selected_lab_is_research_page(lab.page) && !selected_lab_research_projection(&lab, 0, &view));
}

static void malformed_sources(NativeUiContext *context) {
  SelectedLab lab;
  DeviceKit kit;
  LabResearchView view;
  fixture(&lab, &kit);
  lab.game.sample_count = GAME_MAX_SAMPLES + 1;
  assert(!selected_lab_research_projection(&lab, 0, &view));
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  lab.focus = selected_lab_options(&lab);
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  lab.page = V1_STUDY_REVIEW;
  lab.sample = GAME_MAX_SAMPLES;
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  lab.page = V1_FINDING;
  lab.study = PIP_DISCOVERY_METHOD_COUNT;
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  memset(lab.game.samples[0].id, 'X', sizeof(lab.game.samples[0].id));
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  memset(lab.game.sample_metadata[1].content_version, 'X', sizeof(lab.game.sample_metadata[1].content_version));
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  lab.game.sample_metadata[0].disclosed_candidates = 3; /* Unsupported early disclosure. */
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  lab.game.samples[2].decoded_studies = 32;
  assert_failed_routes(&kit, context);
  fixture(&lab, &kit);
  memset(lab.message, 'X', sizeof(lab.message));
  assert_failed_routes(&kit, context);
}

static void assert_native_portrait(const uint8_t *frame, CoreArtId id, unsigned x, unsigned y) {
  const CoreArtSprite *source = core_art_sprite(id);
  assert(source && source->width == 261 && source->height == 289);
  unsigned checked = 0;
  for (unsigned row = 0; row < source->height; ++row)
    for (unsigned column = 0; column < source->width; ++column) {
      const uint8_t *pixel = source->rgba + (row * source->width + column) * 4;
      if (pixel[3] == 255) {
        assert(!memcmp(frame + ((y + row) * SELECTED_LAB_WIDTH + x + column) * 3, pixel, 3));
        ++checked;
      }
    }
  assert(checked > 1000);
}

static void retained_routes_and_exports(void) {
  SelectedLab lab;
  DeviceKit kit;
  NativeUiContext *context = native_ui_create_device(KIT_LAB);
  assert(context);
  const char *names[] = {"collection", "unknown-sample", "topic", "review",
      "partial-p", "incomplete-coat", "complete-coat", "complete-B", "legacy",
      "library", "library-finding", "used-record", "storage-error", "empty-library", "maximum-library"};
  for (unsigned index = 0; index < sizeof(names) / sizeof(names[0]); ++index) {
    fixture(&lab, &kit);
    if (index == 1) lab.focus = 1;
    if (index == 2) lab.page = V1_STUDIES;
    if (index == 3 || index == 12) lab.page = V1_STUDY_REVIEW;
    if (index >= 4 && index <= 8) lab.page = V1_FINDING;
    if (index == 4) assert(pip_record_investigation(&lab.game, 0, 0));
    if (index == 5) { assert(pip_record_investigation(&lab.game, 0, 2)); lab.study = 2; }
    if (index == 6 || index == 10 || index == 11) { complete_a(&lab); lab.study = 2; }
    if (index == 7) { complete_b(&lab); lab.sample = 1; lab.study = 2; }
    if (index == 8) {
      lab.sample = 2;
      lab.game.samples[2].decoded_studies = lab.game.samples[2].decoded_facts = 1;
    }
    if (index == 9) { complete_a(&lab); complete_b(&lab); lab.page = V1_LIBRARY; lab.focus = 5; }
    if (index == 10) lab.page = V1_LIBRARY_FINDING;
    if (index == 11) { lab.page = V1_FINDING; lab.game.samples[0].incubated = 1; }
    if (index == 12) {
      static const char feedback[] = "Storage unavailable. Findings are kept. Retry after recovery; your sample and discoveries stay.";
      assert(sizeof(feedback) == sizeof(lab.message)); /* 95 visible bytes + terminator. */
      lab.storage_error = 1;
      lab.game.data = lab.game.energy = lab.game.essence = 1000000;
      strcpy(lab.message, feedback);
    }
    if (index == 13) { lab.page = V1_LIBRARY; lab.game.sample_count = 0; }
    if (index == 14) {
      lab.page = V1_LIBRARY;
      lab.game.sample_count = GAME_MAX_SAMPLES;
      lab.focus = LAB_RESEARCH_OPTIONS - 1;
      for (unsigned sample = 0; sample < GAME_MAX_SAMPLES; ++sample) {
        memset(&lab.game.sample_metadata[sample], 0, sizeof(lab.game.sample_metadata[sample]));
        memset(lab.game.samples[sample].id, 'S', sizeof(lab.game.samples[sample].id) - 1);
        lab.game.samples[sample].id[39] = 0;
        lab.game.samples[sample].id[38] = (char)('0' + sample);
        snprintf(lab.game.samples[sample].origin_expedition_id,
            sizeof(lab.game.samples[sample].origin_expedition_id), "fixture-max-origin-%u", sample);
        lab.game.samples[sample].decoded_studies = lab.game.samples[sample].decoded_facts = 31;
        lab.game.samples[sample].supported_candidates = PIP_SAMPLE_CANDIDATE_MASK;
      }
    }
    LabResearchView view;
    assert(selected_lab_research_projection(&lab, 0, &view) && native_ui_research(context, &view));
    if (index == 12) assert(view.stock[0] == 10000 && !strcmp(view.message, lab.message));
    if (index == 6) {
      const uint8_t *frame = native_ui_research(context, &view);
      assert(frame);
      assert_native_portrait(frame, CORE_ART_PIP_PLAIN, 405, 247);
      assert_native_portrait(frame, CORE_ART_PIP_MARKED, 699, 247);
    }
    assert_routes(&kit, context, names[index]);
  }
  fixture(&lab, &kit);
  complete_a(&lab);
  lab.page = V1_FINDING;
  lab.study = 2;
  LabResearchView view, invalid;
  assert(selected_lab_research_projection(&lab, 0, &view));
  invalid = view;
  invalid.page = (LabResearchPage)-1;
  assert(!native_ui_research(context, &invalid));
  invalid = view;
  invalid.option_count = LAB_RESEARCH_OPTIONS + 1;
  assert(!native_ui_research(context, &invalid));
  invalid = view;
  invalid.focus = invalid.option_count;
  assert(!native_ui_research(context, &invalid));
  invalid = view;
  memset(invalid.finding, 'X', sizeof(invalid.finding));
  assert(!native_ui_research(context, &invalid));
  invalid = view;
  invalid.topic_count = LAB_RESEARCH_TOPICS + 1;
  assert(!native_ui_research(context, &invalid));
  invalid = view;
  invalid.topic_known[0] = 2;
  assert(!native_ui_research(context, &invalid));
  invalid = view;
  invalid.complete = 0;
  assert(!native_ui_research(context, &invalid));
  invalid = view;
  invalid.partial_p = 1;
  assert(!native_ui_research(context, &invalid));
  invalid = view;
  invalid.portraits[1] = (LabResearchPortrait)99;
  assert(!native_ui_research(context, &invalid));
  invalid = view;
  invalid.page = LAB_RESEARCH_LIBRARY;
  assert(!native_ui_research(context, &invalid));
  malformed_sources(context);
  native_ui_destroy(context);
}

static void physical_review_and_library(void) {
  char directory[] = "/tmp/bee-research-XXXXXX";
  assert(mkdtemp(directory));
  char path[512];
  snprintf(path, sizeof(path), "%s/state", directory);
  SelectedLab lab;
  selected_lab_init(&lab);
  assert(selected_lab_load(&lab, path, 100));
  lab.game.data = lab.game.energy = lab.game.essence = 2000;
  strcpy(lab.game.expedition_id, "research-controls-intake");
  lab.game.expedition_elapsed = GAME_EXPEDITION_SECONDS;
  GameCommand unload = {0};
  unload.type = GAME_COMMAND_EXPEDITION_UNLOAD;
  unload.operation_id = "research-controls-intake";
  unload.sequence = lab.game.last_operation_sequence + 1;
  assert(game_apply(path, &lab.game, &unload) == GAME_OK);
  assert(lab.game.sample_count == 1);
  GameState retained = lab.game;
  button(&lab, SELECTED_RESEARCH_DOWN);
  button(&lab, SELECTED_DOWN_DOWN);
  button(&lab, SELECTED_CONFIRM_DOWN);
  assert(lab.page == V1_STUDIES && !memcmp(&retained, &lab.game, sizeof(retained)));
  button(&lab, SELECTED_RIGHT_DOWN);
  assert(lab.page == V1_STUDIES && !memcmp(&retained, &lab.game, sizeof(retained)));
  button(&lab, SELECTED_CONFIRM_DOWN);
  assert(lab.page == V1_STUDY_REVIEW && !memcmp(&retained, &lab.game, sizeof(retained)));
  LabResearchView view;
  assert(selected_lab_research_projection(&lab, 0, &view) && view.costs[0] == 4);
  NativeUiContext *context = native_ui_create_device(KIT_LAB);
  assert(context && native_ui_research(context, &view));
  assert(!memcmp(&retained, &lab.game, sizeof(retained)));
  button(&lab, SELECTED_BACK_DOWN);
  assert(lab.page == V1_STUDIES && !memcmp(&retained, &lab.game, sizeof(retained)));
  button(&lab, SELECTED_CONFIRM_DOWN);
  button(&lab, SELECTED_CONFIRM_DOWN);
  assert(lab.page == V1_FINDING && lab.game.data == retained.data - 400);
  assert(lab.game.last_operation_sequence == retained.last_operation_sequence + 1);
  assert(selected_lab_research_projection(&lab, 0, &view) && view.partial_p && !view.complete);
  GameState investigated = lab.game;
  unsigned stale = lab.revision - 1;
  selected_lab_input(&lab, SELECTED_CONFIRM_UP, 0, stale);
  assert(!memcmp(&investigated, &lab.game, sizeof(investigated)));
  button(&lab, SELECTED_LIBRARY_DOWN);
  assert(lab.page == V1_LIBRARY && selected_lab_options(&lab) == 1);
  button(&lab, SELECTED_RIGHT_DOWN);
  assert(lab.page == V1_LIBRARY_FINDING);
  assert(selected_lab_research_projection(&lab, 0, &view) && view.known_method && !view.portraits[0]);
  assert(native_ui_research(context, &view));
  button(&lab, SELECTED_BACK_DOWN);
  assert(lab.page == V1_LIBRARY);
  button(&lab, SELECTED_HOME_DOWN);
  assert(lab.page == V1_HOME && !memcmp(&investigated, &lab.game, sizeof(investigated)));
  SelectedLab reloaded;
  selected_lab_init(&reloaded);
  assert(selected_lab_load(&reloaded, path, 100));
  SelectedResearchView knowledge;
  assert(selected_lab_research_view(&reloaded, 0, &knowledge) && knowledge.partial_p && !knowledge.complete);
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
  complete_a(&lab);
  NativeUiContext *companion = native_ui_create(), *dock = native_ui_create_device(KIT_DOCK);
  assert(companion && dock);
  CompanionResidentView resident = {0};
  resident.portrait = RESIDENT_PORTRAIT_PLAIN;
  resident.count = resident.current = resident.online = 1;
  strcpy(resident.identity, "fixture-research-peer");
  DockView dock_view;
  assert(native_ui_resident(companion, &resident));
  assert(kit_dock_projection(&kit, &dock_view) && native_ui_dock(dock, &dock_view));
  NativeUiContext *context = native_ui_create_device(KIT_LAB);
  assert(context && ui_display_count() == 3);
  LabHomeView home;
  lab.page = V1_HOME;
  assert(selected_lab_home_view(&lab, NULL, 0, &home));
  LabReceptionView reception;
  lab.page = V1_EXPEDITION;
  assert(kit_reception_projection(&kit, &reception));
  LabResearchView pair, topic;
  lab.page = V1_FINDING;
  lab.study = 2;
  assert(selected_lab_research_projection(&lab, 0, &pair));
  lab.page = V1_STUDIES;
  lab.focus = 0;
  assert(selected_lab_research_projection(&lab, 0, &topic));
  lv_mem_monitor_t first, final;
  for (unsigned cycle = 0; cycle < 200; ++cycle) {
    assert(native_ui_home(context, &home) && native_ui_reception(context, &reception));
    assert(native_ui_research(context, cycle % 2 ? &topic : &pair));
    assert(native_ui_resident(companion, &resident) && native_ui_dock(dock, &dock_view));
    if (cycle == 99) lv_mem_monitor(&first);
  }
  lv_mem_monitor(&final);
  printf("Research/reception/Home plus Companion resident/Dock: used=%zu peak=%zu total=%zu first_free=%zu final_free=%zu\n",
      final.total_size - final.free_size, final.max_used, final.total_size, first.free_size, final.free_size);
  fflush(stdout);
  assert(first.free_size == final.free_size);
  native_ui_destroy(context);
  assert(ui_display_count() == 2 && native_ui_resident(companion, &resident) && native_ui_dock(dock, &dock_view));
  context = native_ui_create_device(KIT_LAB);
  assert(context && native_ui_research(context, &pair));
  assert(native_ui_resident(companion, &resident) && native_ui_dock(dock, &dock_view));
  native_ui_destroy(context);
  native_ui_destroy(dock);
  native_ui_destroy(companion);
  assert(ui_display_count() == 0);
}

int main(void) {
  projection_cases();
  retained_routes_and_exports();
  physical_review_and_library();
  three_context_memory();
  puts("Lab research disclosure, native routes, physical authority and retained lifetime checks passed");
  return 0;
}
