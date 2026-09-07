#include "setup.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "hal/uart_types.h"
#include "nvs_flash.h"
#include "receive_data.h"
#include "driver/uart.h"
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

//we gonna initialize uart
void init_uart(void) {
  //first we start uart buffered io with event queue

  const int uart_buffer_size = (1024*2);
  QueueHandle_t uart_queue;

  //install uart driver using an event queue
  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0) );

  //set up the communication parameters
  uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
    .rx_flow_ctrl_thresh = 122,
  };


  //we need to set the uart pins 

  //pin 41 - rx
  //pin 42 - tx
  ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, 42, 41, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    

  


}

//then in order to send data we follow these steps 
  //1. write data into the transmit (TX) data buffer
  //2. FSM serializes the data (meaning that converts the data into a sequential sequence of bits )
  //3. FSM sends the data from the TX Buffer into the Transmit Shift Register (TSR), once transferred
  //the TX buffer is empty and ready for more data.

  //receiving data
  //1. fsm processes an incoming serial stream of data bits and parallizes it (which means incoming bits, 
  //are placed into the Receive Shift Register (RSR) one bit at a time then once all 8 bits have arrived including parity and stop bit
  //the full 8 bit chunk is reconstructed into a single parallel byte)
  //2. fsm writes the data into RX FIFO buffer 
  //3. reads data from the RX FIFO buffer.

  //so an application only reads and writes data from a specific buffer using uart_write_bytes() and uart_read_bytes()






//functions for configuring specific parameters individually 

  //uart_set_pin - assigns esp32 gpio to uart hardware

  //esp_err_t uart_set_pin (uart_port_t uart_num, int tx_io_num, int rx_io_num, rts_io_num, cts_ionum)

  //rts - flow control: ready to send
  ///cts - flow control: ready to send




  //baud rate 
  //uart_set_baudrate()
  //uart_set_baudrate(uart_port_t uart_num, uint32_t baud_rate)

  //number of transmitted bits
  //uart_set_word_length() selected out of uart_word_length_t
  //uart_set_word_length(uart_port_t uart_num, uart_word_length_t data_bit)
  //uart_word_length_t could be 
  //UART_DATA_5_BITS (5 data bits)
  //UART_DATA_6_BITS (6 data bits)
  //UART_DATA_7_BITS (7 data bits)
  //UART_DATA_8_BITS (Standard / most common)


  //parity control (means error checking bit appended to each data byte)
  //uart_set_parity() selected out of uart_parity_t
  //uart_set_parity(uart_port_t uart_num, uart_parity_t parity_mode)
  //uart_parity_t can be
  //UART_PARITY_DISABLE (no parity bit)
  //UART_PARITY_EVEN (parity bit ensures an even number of 1s)
  //UART_PARITY_ODD (parity bit ensures an odd number of 1s)

  //hardware flow control mode 
  //uart_set_hw_flow_ctrl() selected out of uart_hw_flowcontrol_t 
  //uart_set_hw_flow_ctrl(uart_port_t uart_num, uart_hw_flowcontrol_t flow_ctrl, uint8_t rx_thresh)
  //Meaning: Uses extra physical pins (RTS - Ready to Send, CTS - Clear to Send) 
  //to prevent the sender from transmitting faster than the receiver's buffer can handle.

  //Selected out of uart_hw_flowcontrol_t:
  //UART_HW_FLOWCTRL_DISABLE (Standard / no hardware handshaking)
  //UART_HW_FLOWCTRL_RTS (Only enable RTS)
  //UART_HW_FLOWCTRL_CTS (Only enable CTS)
  //UART_HW_FLOWCTRL_CTS_RTS (Enable full hardware flow control)

  //communication mode
  //uart_set_mode() selected out of uart_mode_t 
  //Selects the electrical / protocol mode of the UART hardware 
  // (standard UART vs half-duplex RS485 vs Infrared).

  //Selected out of uart_mode_t:
  ///UART_MODE_UART (Standard full-duplex UART)
  //UART_MODE_RS485_HALF_DUPLEX (RS485 half-duplex, automatically toggles RTS/direction pin)
  //UART_MODE_IRDA (Infrared communication)
  //UART_MODE_RS485_COLLISION_DETECT
  //UART_MODE_RS485_APP_CTRL

    //const uart_port_t uart_num = UART_NUM_1;
  // Setting parameters individually:
  //uart_set_baudrate(uart_num, 115200);
  //uart_set_word_length(uart_num, UART_DATA_8_BITS);
  //uart_set_parity(uart_num, UART_PARITY_DISABLE);
  //uart_set_hw_flow_ctrl(uart_num, UART_HW_FLOWCTRL_DISABLE, 0);
  //uart_set_mode(uart_num, UART_MODE_UART);