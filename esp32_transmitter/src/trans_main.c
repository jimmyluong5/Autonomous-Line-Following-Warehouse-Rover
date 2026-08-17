#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "servo.h"
#include "setup.h"
#include "transmit_data.h"
#include "uart_control.h"

void app_main() {
  // Initialize UART console & Servo FIRST before any network/WiFi setup
  UART_CONTROL_init();
  servo_init();

  init_led();
  init_esp_nvs();
  init_wifi();
  init_esp_now();
  init_button_pin();

  while (1) {
    UART_CONTROL_update();
    Servo_Update();

    uint8_t packet = read_buttons();
    transmit_data(receiver_mac, packet);

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
