#include "demo_domain.h"
#include "demo_pixels.h"
#include <zephyr/sys/printk.h>

int main(void) {
  printk("Critter Probe native scaffold 0.1.0 | target: xiao_ble/nrf52840\n");
  Demo demo;
  uint8_t row[250 * 3];
  unsigned checksum = 0;
  demo_init(&demo);
  const char *failure = demo_apply(&demo, "select");
  if (failure || !demo_valid(&demo)) {
    printk("Shared fixture transition validation failed\n");
    return 1;
  }
  demo_row(&demo, 1, 10, row);
  for (unsigned i = 0; i < sizeof(row); ++i)
    checksum += row[i];
  printk("Shared fixture selected: %u; row checksum: %u (no panel attached)\n",
         demo.selected, checksum);
  return 0;
}
