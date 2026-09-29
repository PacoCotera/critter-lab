#ifndef BEECHO_KIT_H
#define BEECHO_KIT_H
#include "selected_lab.h"

enum { KIT_LAB, KIT_COMPANION, KIT_DOCK, KIT_DEVICE_COUNT };
enum {
  KIT_IDLE,
  KIT_WAITING,
  KIT_ARRIVED,
  KIT_COMMITTING,
  KIT_ACK_PENDING,
  KIT_COMPLETE
};
enum { COMP_PROBE, COMP_CARGO, COMP_FRIENDS, COMP_MODES, COMP_SEND_REVIEW };

typedef struct {
  unsigned revision, acknowledged, epoch, acknowledged_epoch, focus, page;
  int suspended;
  SelectedGesture gestures[10];
  char message[96];
} KitView;

/* Host simulation journal; not a production radio packet or MCU save format. */
typedef struct {
  uint32_t checksum, version, phase, companion_online, dock_online;
  uint64_t accept_sequence;
  char haul_id[64];
  uint32_t cargo[3], elapsed, kind;
  uint32_t dock_stock[3], dock_samples, dock_residents, dock_incubations;
  uint64_t dock_world_revision, dock_updated_at;
} KitJournal;

typedef struct {
  SelectedLab *lab;
  KitView companion, dock;
  KitJournal journal;
  char journal_path[560];
  int failed;
  uint32_t clock, next_delivery, dock_updated;
} DeviceKit;

int kit_init(DeviceKit *kit, SelectedLab *lab, uint32_t clock);
void kit_tick(DeviceKit *kit, uint32_t clock);
void kit_input(DeviceKit *kit, unsigned device, SelectedInput input,
               unsigned revision);
int kit_link(DeviceKit *kit, unsigned device, int online);
void kit_status(DeviceKit *kit, unsigned device, FILE *output);
int kit_bmp(const DeviceKit *kit, unsigned device, FILE *output);
unsigned kit_revision(const DeviceKit *kit, unsigned device);
unsigned kit_width(unsigned device);
unsigned kit_height(unsigned device);
const char *kit_option(const DeviceKit *kit, unsigned device, unsigned index);
const char *kit_stage(const DeviceKit *kit);
int kit_lab_explore(const DeviceKit *kit);
#endif
