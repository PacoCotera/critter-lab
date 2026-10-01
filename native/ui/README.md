# Retained device UI boundary

The complete host Dock page family uses `dock_ui.c`: World, Supplies,
Connections, opened pages, print review, cached/offline and storage errors.
It creates retained LVGL labels, image widgets, buttons and focus; it does not
display an earlier raster renderer as an image. `selected-lab/dock_view.c`
copies the accepted Kit snapshot, physical focus, actions and timestamp into
the plain `DockView`. UI code cannot invoke a game command or access a Kit.
Each update copies the view into owned storage before setting static labels.

`display.c` shares one process-wide LVGL lifetime across Companion and Dock.
A profile defines native dimensions, RGB888 format and partial draw rows.
The caller supplies a bounded draw buffer. The callback validates the area,
stride and buffer size before passing it to a sink. RGB888 bytes supplied by
LVGL are B,G,R in this configuration; `host_frame.c` copies them into RGB.
Companion uses 450x600 with 60 draw rows; Dock uses 792x272 with 60 rows.
Buffer planning includes the configured LVGL stride alignment.

A synchronous sink returns `UI_FLUSH_COMPLETE`. An asynchronous sink returns
`UI_FLUSH_PENDING` and later calls `ui_display_flush_complete`; it must finish
before destroying the display or releasing its draw buffer. Completion releases
the buffer for reuse. It does **not** establish a visible frame. A target may
separately call `ui_display_mark_visible` after its presentation acknowledgement.
The host Kit protocol continues using its existing revision/epoch READY guard;
rendering or exporting a BMP does not send READY.

Full RGB storage lives in the optional host frame adapter because BMP export
needs a complete image. A partial sink does not need that allocation. Dock's
four-gray conversion (0/85/170/255) happens in the host export adapter after
LVGL rendering. No UI widget or domain quantity is quantized. This is host
software evidence, not ESP32 display, radio, printer, charging or panel proof.
The current host adapter completes synchronously; no device driver is present.

`native_ui_create()` preserves Companion callers. `native_ui_create_device`
creates only the applicable Companion or Dock roots and assets. The application
keeps one context per migrated device. Controlled Cargo animation refuses to
advance while another display exists because LVGL's clock is global. These
bounded contexts and fixed label storage do not promise recovery from arbitrary
allocation failure inside upstream LVGL.

The existing `dock_four_gray_checks` CTest covers copied snapshot authority,
all Dock pages and unavailable states, repeated retained updates, shared
lifetime, partial area/stride/channel handling and asynchronous buffer release
without automatic visibility. `dock_gray_checks OUTPUT_DIRECTORY` also writes
labeled representative BMP fixtures; these are synthetic presentation evidence,
not a physical-control playthrough. Run through the project's native build gate.

The existing Cargo tree is extracted into `companion_cargo_ui.c` and shared by
Cargo and Send/Keep. Its plain copied view includes a screen tag and projected
footer, exact units, sample and focus. Borrowed fonts/images remain caller-owned;
the module owns widgets and retained label storage. The module accepts a partial
display sink without allocating host full-frame storage. Kit projection and
physical command authority remain outside it. Offline Send seals locally and
waits; Lab acceptance clears current cargo and ends the outing. A defensive
already-sealed review presents no Keep cancellation or second Send action.
The [actual native confirmation/return proof](../../docs/evidence/native-companion-send/README.md)
passed affected checks and independent technical/focused UI/UX output review.
Current Companion ESP-IDF compilation and physical output remain separate gates.

Lab and the remaining Companion Discard/Finish/preview/visit routes remain open
renderer migrations. A failed migrated Dock/Probe/Cargo/Send projection returns a
render error; it cannot silently reach the old manual renderer.
