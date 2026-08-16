// accept an input data from a button click, but that button is pressed on the
// esp32 send it via ESP-NOW, the esp32 receives the data, it will then send it
// to the LED to blink it.

#include <stdio.h>
#include <stdlib.h>
#include <esp_now.h>
#include <transmit.data.h>
#include <stdint.h>
#include <string.h>
#include <driver/gpio.h>
#include "transmit_data.h"
//need to make the data packet struct

typedef struct {
    //button data for the controls
    uint8_t button_data;

    //data for the fpga might need to make this 16 bits.
    uint8_t fpga_data;
    uint8_t speed;
    uint16_t sequence;


} data_packet;

//later we can declare a general data packet which contains all of these attributes.
//example: data_packet packet = ...
//packet.fpga_data = blah blah
//or 
//packet.speed = 100, 

//gpio_state
// 0000_0000, 
// lsb is the button state, 0 for LOW, 1 for high

//so i click the push button, read the state of the button from the esp32 pin, 
// if readPin(pin_number) is LOW, then set the right most bit to 1. (1<<0), 
//the button is active low, so we will do this, if readPin is low, button is pressed, and set the right most bit
//else, the readPin is high and the bit will be set low.

//need to initalize one of the pins as an input.
//use a function for this. 

//to initialize the pin its just 
//gpio_set_direction(GPIO_NUM_x, GPIO_MODE_INPUT)
//where x is whatever button pin you are using.
//I am going to use GPIO_NUM_1

//gpio_get_level(GPIO_NUM_X); reads voltage from a pin, and decides
//to output 0 or 1

//gpio_set_level(GPIO_NUM_x), sets voltage for a pin, writes 1 or 0

// 2. WRITE voltage to turn LED ON (3.3V)
//gpio_set_level(GPIO_NUM_2, 1);
// 3. WRITE voltage to turn LED OFF (0V)
//gpio_set_level(GPIO_NUM_2, 0);


//this file has two jobs
//1. read the button press
//2. create the data packet.

void init_button_pin(void) {
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);
}

uint8_t read_buttons(void) {
    uint8_t data_packet = 0b00000000;

    //read the button press
    int button_press = gpio_get_level(GPIO_NUM_1);

    //then if the button_press is 0 then we have a valid button press
    if (button_press == 0) {
        //set the LSB to 1
        data_packet = data_packet | (1<<0);
    }
    else {
        data_packet = 0b00000000;
    }
    return data_packet;
}

