#include "setup.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "ESP32_TRANSMITTER";

// Broadcast address: sends to any nearby receiver
static uint8_t receiver_mac[ESP_NOW_ETH_ALEN] = {0xFF, 0xFF, 0xFF,
                                                 0xFF, 0xFF, 0xFF};

static void OnDataSent(const esp_now_send_info_t *tx_info,
                       esp_now_send_status_t status) {
  ESP_LOGI(TAG, "Packet send status to MAC " MACSTR ": %s",
           MAC2STR(tx_info->des_addr),
           status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void init_esp_nvs(void) {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
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
  ESP_ERROR_CHECK(esp_now_register_send_cb(OnDataSent));

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiver_mac, ESP_NOW_ETH_ALEN);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;

  if (!esp_now_is_peer_exist(receiver_mac)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to add peer");
    }
  }
}

