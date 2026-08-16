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

#define LED_PIN GPIO_NUM_1

typedef struct{
    uint8_t button_data;
    uint8_t fpga_data;
    uint8_t speed;
    uint16_t sequence;
} data_packet;

//need to initialize pin

//this file has 



void init_led(void){
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0); //the led starts off.
}

//we receive input data in the form of the data packet, then output a 1 or 0 and give it to the LED
void receive_button_press(uint8_t data) {
    //we read the data using bitwise and
    
    //if data = 0000_0001 then it'll be 1
    if ((data & (1<<0)) == 1) {
        gpio_set_level(LED_PIN, 1);
    }
    else {
        gpio_set_level(LED_PIN,1);
    }

}



