#include "demo_domain.h"
#include "demo_pixels.h"
#include "esp_log.h"

void app_main(void) {
  ESP_LOGI("critter_companion", "Native scaffold 0.1.0 | target: esp32s3");
  Demo demo;
  uint8_t row[250 * 3];
  unsigned checksum = 0;
  demo_init(&demo);
  const char *failure = demo_apply(&demo, "select");
  if (failure || !demo_valid(&demo)) {
    ESP_LOGE("critter_companion", "Shared fixture transition validation failed");
    return;
  }
  demo_row(&demo, 1, 10, row);
  for (unsigned i = 0; i < sizeof(row); ++i)
    checksum += row[i];
  ESP_LOGI("critter_companion",
           "Shared fixture selected: %u; row checksum: %u (no panel attached)",
           demo.selected, checksum);
}
