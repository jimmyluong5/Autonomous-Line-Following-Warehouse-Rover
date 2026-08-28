#include "setup.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "receive_data.h"

//static const char *TAG = "ESP32_RECEIVER";

//this is the esp-now callback, which is called automatically whenever the receiver esp32 
//gets an esp-now packet.


//callback function for the esp32-now communication protocol

//like when the esp32 receives data via esp-now, the system automatically interrupts
//what its doing then calls this function to handle the incoming data.

//static restricts this variable to this file, so it cannot be a naming conflict with other files
//const esp_now_recv_info_t *esp_now_info is a pointer to a structure containing
//meta data about the receiving data, contains the sender's MAC address, the destination MAC address 

//const uint8_t *data - pointer to the memory address of the data sent by the other device.

static void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len) {



  //ion even need this hoe.
  //ESP_LOGI(TAG, "Received %d bytes from MAC: " MACSTR, data_len,MAC2STR(esp_now_info->src_addr));

  //make sure the length is exactly the length of data_packet_t which is 6 bytes.

  //also guarantees that any incoming data is exactly our data packet that we made.
  if (data_len == sizeof(data_packet_t)) { 
    //local variable that we have created for our incoming data.
    data_packet_t packet; //initalize the packet to place our data in.
    
    //copy the data into data_packet_t packet

    //destination - &packet (where the data is going which is in the mem address of the packet.)
    //data -contains the memory address of the temporary raw byte buffer managed by the ESP-NOW Wi-Fi driver. 
      //think as we send the raw bytes through wifi, the raw bytes are stored in RAM at a particular memory address
      //data contains that points to the that memory address where the raw bytes/data is sent.
    //sizeof(Data_packet_t) //number of bytes to copy.
    memcpy(&packet, data, sizeof(data_packet_t)); 
    receive_button_press(&packet);
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



