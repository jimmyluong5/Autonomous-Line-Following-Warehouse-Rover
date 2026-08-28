#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "setup.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "receive_data.h"
#include "driver/gpio.h"

void app_main() {
  init_led();
  init_esp_nvs();
  init_wifi();
  init_pins();

  //since the i got the mac address of this esp32 ion need it no more.
  //need to get the MAC address of this esp32 and put it in the transmitter's esp32.
  //uint8_t mac[6];
  //esp_read_mac(mac, ESP_MAC_WIFI_STA);
  //ESP_LOGI("MAC_ADDRESS", "Receiver MAC: " MACSTR, MAC2STR(mac)); //prints this in the serial 

  
  init_esp_now(); //registers the OnDataRecv callback
  while (1) {
    //gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
    //since i have the mac address ion need this no more
    //ESP_LOGI("MAC_ADDRESS", "Receiver MAC: " MACSTR, MAC2STR(mac)); //constantly prints this message
    vTaskDelay(pdMS_TO_TICKS(2000));
    //gpio_set_level(GPIO_NUM_5, 1);
    
    //vTaskDelay(pdMS_TO_TICKS(200));

    //gpio_set_level(GPIO_NUM_5, 0);
    //vTaskDelay(pdMS_TO_TICKS(200));
  }
}
