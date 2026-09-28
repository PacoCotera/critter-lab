#include "selected_lab.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static int number(const char *value, unsigned *result) {
  char *end;
  unsigned long parsed = strtoul(value, &end, 10);
  if (!*value || *end || *value == '-' || parsed > UINT_MAX) return 0;
  *result = (unsigned)parsed;
  return 1;
}

static void status(const SelectedLab *lab) {
  printf("{\"revision\":%u,\"page\":\"%s\",\"focus\":\"%s\",\"ready\":%s,\"suspended\":%s,\"width\":1024,\"height\":600,\"stock\":[2,1,0],\"boundary\":\"Native preview; research is not connected\"}\n", lab->revision, selected_lab_page(lab), selected_lab_focus(lab), lab->ready ? "true" : "false", lab->suspended ? "true" : "false");
}

static int event(const char *name, SelectedInput *input) {
  static const char *const names[] = {"rotate", "confirm-down", "confirm-up", "back-down", "back-up", "cancel", "suspend", "resume", "ready"};
  for (unsigned index = 0; index < sizeof(names) / sizeof(names[0]); ++index)
    if (!strcmp(name, names[index])) { *input = (SelectedInput)index; return 1; }
  return 0;
}

int main(int argc, char **argv) {
  SelectedLab lab;
  selected_lab_init(&lab);
  if (argc == 3 && !strcmp(argv[1], "frame")) {
    FILE *output = fopen(argv[2], "wb");
    if (!output) return 2;
    int success = selected_lab_bmp(&lab, output);
    if (fclose(output)) success = 0;
    return success ? 0 : 2;
  }
  if (argc != 2 || strcmp(argv[1], "serve")) {
    fprintf(stderr, "Usage: selected_lab frame OUTPUT.bmp | selected_lab serve\n");
    return 2;
  }
  char line[128];
  while (fgets(line, sizeof(line), stdin)) {
    char name[32], argument[32], frame_argument[32], extra[2];
    int count = sscanf(line, "%31s %31s %31s %1s", name, argument, frame_argument, extra);
    unsigned frame = 0;
    if (count == 1 && !strcmp(name, "status")) status(&lab);
    else if (count == 2 && !strcmp(name, "frame") && number(argument, &frame)) {
      if (frame != lab.revision) puts("{\"error\":\"Stale frame request\"}");
      else {
        printf("{\"revision\":%u,\"bytes\":%u}\n", lab.revision, 54u + SELECTED_LAB_WIDTH * SELECTED_LAB_HEIGHT * 3u);
        if (!selected_lab_bmp(&lab, stdout)) return 2;
      }
    } else {
      SelectedInput input;
      int valid = count >= 2 && event(name, &input);
      int delta = 0;
      if (valid && input == SELECTED_ROTATE) {
        valid = count == 3 && (!strcmp(argument, "1") || !strcmp(argument, "-1")) && number(frame_argument, &frame);
        delta = argument[0] == '-' ? -1 : 1;
      } else valid = valid && count == 2 && number(argument, &frame);
      if (!valid) puts("{\"error\":\"Unsupported input\"}");
      else { selected_lab_input(&lab, input, delta, frame); status(&lab); }
    }
    if (fflush(stdout)) return 2;
  }
  return ferror(stdin) ? 2 : 0;
}
