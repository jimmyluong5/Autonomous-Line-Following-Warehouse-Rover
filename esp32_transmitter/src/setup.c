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

//this should be the mac of the 2nd esp32 (receiver).
//so basically we need to find it's mac address and put it here.
//Media Access Control (MAC) - address is a unique 12-digit 
//code used to identify a device on a network.
uint8_t receiver_mac[ESP_NOW_ETH_ALEN] = {0xAC, 0x27, 0x6E, 0xA2, 0x87, 0x5C};

static void OnDataSent(const esp_now_send_info_t *tx_info,
                       esp_now_send_status_t status) {
  if (tx_info == NULL) return;
  // Silenced log to avoid flooding UART terminal output
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
  ESP_ERROR_CHECK(esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));
}

void init_esp_now(void) {
  ESP_ERROR_CHECK(esp_now_init()); //this returns an esp_err_t, which is 
  //an error code. we use this to check if the function was successful.
  ESP_ERROR_CHECK(esp_now_register_send_cb(OnDataSent)); //this also returns an esp_err_t.

  esp_now_peer_info_t peerInfo = {}; //initialize the peer info struct.
  //it's just a struct that holds the information of the peer.

  //this basically copies the data from the receiver_mac array to the 
  //peerInfo.peer_addr array.
  memcpy(peerInfo.peer_addr, receiver_mac, ESP_NOW_ETH_ALEN);
  //we set the channel to 1 because that's what we set the wifi channel to.
  peerInfo.channel = 1;
  //we set encrypt to false because we're not encrypting the data.
  peerInfo.encrypt = false;

  //we check if the peer exists, if it does not, we add it.
  if (!esp_now_is_peer_exist(receiver_mac)) {
    //this also returns an esp_err_t.
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to add peer"); //if it fails to add peer, it'll print this. 
    }
  }
}



