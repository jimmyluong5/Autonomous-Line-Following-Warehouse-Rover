// receive data, and depending on what the action is we do something.
//like the goal is to turn on an LED from the other esp32 button click. 
#include "receive_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <esp_now.h>
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





static uint8_t s_last_state = 0xFF; // track previous button state

//we receive input data in the form of the data packet, then output a 1 or 0 and give it to the LED
void receive_button_press(uint8_t data) {
    uint8_t current_state = (data & (1 << 0));

    // only update GPIO level if state actually changed
    if (current_state != s_last_state) {
        s_last_state = current_state;

        if (current_state != 0) {
            gpio_set_level(LED_PIN, 1); // Turn LED ON
        } else {
            gpio_set_level(LED_PIN, 0); // Turn LED OFF
        }
    }
}



