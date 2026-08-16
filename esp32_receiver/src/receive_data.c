// receive data, and depending on what the action is we do something.
//like the goal is to turn on an LED from the other esp32 button click. 
#include "receive_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <esp_now.h>
#include <transmit.data.h>
#include <stdint.h>
#include <string.h>
#include <driver/gpio.h>

typedef struct{
    uint8_t button_data;
    uint8_t fpga_data;
    uint8_t speed;
    uint16_t sequence;
} data_packet;

//need to initialize pin

void init_led(void){

}