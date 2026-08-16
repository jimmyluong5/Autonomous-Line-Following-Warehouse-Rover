#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "setup.h"

void app_main() {
  //init_led();
  init_esp_nvs();
  init_wifi();
  init_esp_now(); //registers the OnDataRecv callback
  while (1) {
    // blink_led();

    //constantly get the packet.
    uint8_t packet = receive_button_press(data);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
