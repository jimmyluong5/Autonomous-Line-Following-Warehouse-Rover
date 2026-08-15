#include "freertos/FreeRTOS.h"
#include "freertos/task.h" //includes FreeRTOS task utilities (which provides a function to pause)
#include "setup.h"

void app_main() {
  init_esp_nvs();
  init_wifi();
  init_esp_now();
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}