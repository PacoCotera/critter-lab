#ifndef DEMO_PIXELS_H
#define DEMO_PIXELS_H
#include "demo_domain.h"
/* RGB scanline: bounded memory even when compiled for a portable MCU. */
void demo_dimensions(int probe, unsigned *width, unsigned *height);
void demo_row(const Demo *demo, int probe, unsigned y, uint8_t *rgb);
#endif
