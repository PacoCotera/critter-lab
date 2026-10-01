#ifndef CRITTER_NATIVE_UI_H
#define CRITTER_NATIVE_UI_H
#include "cargo_view.h"
#include "probe_view.h"

typedef struct NativeUiContext NativeUiContext;
NativeUiContext *native_ui_create(void);
void native_ui_destroy(NativeUiContext *context);
/* RGB byte order matches kit's existing row renderer; NULL means failure. */
const uint8_t *native_ui_cargo(NativeUiContext *context,
                               const CompanionCargoView *view, int still);
const uint8_t *native_ui_probe(NativeUiContext *context, const CompanionProbeView *view);
/* Controlled proof clock only: no game tick or input acknowledgement.
 * Advancement is refused while another context exists: LVGL's clock is global. */
void native_ui_advance(NativeUiContext *context, unsigned milliseconds);
void native_ui_cancel(NativeUiContext *context);
int native_ui_animation_pending(const NativeUiContext *context);
int kit_bmp_ui(const DeviceKit *kit, unsigned device, FILE *output,
               NativeUiContext *context, int still);
#endif
