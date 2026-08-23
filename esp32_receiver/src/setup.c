#include "setup.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "receive_data.h"

static const char *TAG = "ESP32_RECEIVER";

//this is the esp-now callback, which is called automatically whenever the receiver esp32 
//gets an esp-now packet.
static void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len) {
  
  //ion even need this hoe.
  //ESP_LOGI(TAG, "Received %d bytes from MAC: " MACSTR, data_len,MAC2STR(esp_now_info->src_addr));

  //make sure we received at least one byte
  if (data_len > 0) {
    //then place the data into this function.
    receive_button_press(data[0]);
  }
}

void init_wifi(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));
}

void init_esp_now(void) {
  ESP_ERROR_CHECK(esp_now_init());
  ESP_ERROR_CHECK(esp_now_register_recv_cb(OnDataRecv));
}

void init_esp_nvs(void) {
     esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}



