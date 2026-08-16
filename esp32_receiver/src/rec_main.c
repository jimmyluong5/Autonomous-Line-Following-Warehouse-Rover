#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "setup.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "receive_data.h"

void app_main() {
  init_led();
  init_esp_nvs();
  init_wifi();

  //need to get the MAC address of this esp32 and put it in the transmitter's esp32.
  uint8_t mac[6];

  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  ESP_LOGI("MAC_ADDRESS", "Receiver MAC: " MACSTR, MAC2STR(mac));



  init_esp_now(); //registers the OnDataRecv callback
  while (1) {
    ESP_LOGI("MAC_ADDRESS", "Receiver MAC: " MACSTR, MAC2STR(mac));
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}
