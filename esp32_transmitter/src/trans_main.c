#include "freertos/FreeRTOS.h"
#include "freertos/task.h" //includes FreeRTOS task utilities (which provides a function to pause)
#include "led.h"
#include "setup.h"
#include "transmit_data.h"



void app_main() {
  init_led();
  init_esp_nvs(); //idk what this is
  init_wifi(); //initialize wifi
  init_esp_now(); //initialize esp_now which is the communication between esp32s.
  init_button_pin(); //gpio 1 is the input.
  while (1) {
    //blink_led();

    //constantly read the button
    uint8_t packet = read_buttons();
    
    //transmit packet over ESP-NOW
    transmit_data(receiver_mac, packet);
    vTaskDelay(pdMS_TO_TICKS(50)); // check 20 times per second
  }
}
