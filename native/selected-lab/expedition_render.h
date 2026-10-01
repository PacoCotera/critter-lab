#ifndef CRITTER_LAB_EXPEDITION_RENDER_H
#define CRITTER_LAB_EXPEDITION_RENDER_H
#include "kit.h"
#include "../ui/expedition_view.h"

int kit_field_projection(const DeviceKit *kit, ExpeditionFieldView *out);
unsigned kit_received_count(const DeviceKit *kit);
int kit_received_projection(const DeviceKit *kit, unsigned index,
                            ExpeditionReceivedView *out);
void expedition_field_row(const ExpeditionFieldView *view, unsigned y,
                          uint8_t *pixels);
void expedition_received_row(const ExpeditionReceivedView *view, unsigned y,
                             uint8_t *pixels);

#endif
