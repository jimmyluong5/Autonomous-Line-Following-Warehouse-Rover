#include "setup.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

static const char *TAG = "ESP32_RECEIVER";

static void OnDataRecv(const esp_now_recv_info_t *esp_now_info,
                       const uint8_t *data, int data_len) {
  ESP_LOGI(TAG, "Received %d bytes from MAC: " MACSTR, data_len,
           MAC2STR(esp_now_info->src_addr));
}

void init_wifi(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
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


