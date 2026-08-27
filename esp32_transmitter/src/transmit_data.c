#include <stdio.h>
#include <stdlib.h>
#include <esp_now.h>
#include <stdint.h>
#include <string.h>
#include <driver/gpio.h>
#include "esp_log.h"
#include "setup.h"
#include "transmit_data.h"
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//static const char *TAG = "TRANSMIT_DATA";

// Array for the GPIO pins to loop through and read

int button_pins[] = {
    GPIO_NUM_10, //0 (left) //manual mode
    GPIO_NUM_11, //idx 1 (down) //decrease speed by 10 % (0-255 then its by 25 counts or 5% = 13 counts)
    GPIO_NUM_12, //idx 2 (up) //increase speed by 10% or 5%
    GPIO_NUM_13, //idx 3 (Stop) //just stop
    GPIO_NUM_14 //idx 4 (right) //autonomous mode.
};


int led_pins[]= {
    GPIO_NUM_18, //left (yellow)
    GPIO_NUM_8, //down (blue)
    GPIO_NUM_3, //up (green)
    GPIO_NUM_46, //stop (red)
    GPIO_NUM_9, //right (white)

};

typedef struct {
    uint8_t button_data;
    uint8_t fpga_data;
    uint8_t speed;
    uint16_t sequence;
} data_packet_t;

void init_button_pin(void) {

    //we need to initialize all the pins to input
    for (int i = 0; i < 5; i++) {
        gpio_set_direction(button_pins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(button_pins[i], GPIO_PULLUP_ONLY);
    }
}

void init_led_pin(void){
    for (int i = 0; i < 5; i++){
        gpio_set_direction(led_pins[i], GPIO_MODE_OUTPUT);
        //set them off at first
        gpio_set_level(led_pins[i], 0);
    }
    
}



uint8_t read_buttons(void) {
    uint8_t data_packet = 0x00; //each 0 is half a byte

//we are going to try a debouncing method with sampling
//where we sample once and see the state of the buttons, then we wait 20ms later and sample again,
//in the end we return the bitwise and between the two samples to ensure that a proper button click happened

    uint8_t sample1 = 0x00; //8 bit
    uint8_t sample2 = 0x00;
    
    //populate sample1 with the data from the buttons
    for (int i = 0; i < 5; i++) {
        if (gpio_get_level(button_pins[i]) == 0) {
            //then just shift the bits and place it into sample1
            sample1 = sample1 | (1<<i);
        }
    }

    //sample1 has the data from the first sampling of the button states

    //now we check if sample1 contains any 1s, which means we have a button press, if not then
    //we just move to the 2nd sample
    
    //if it does then we have to delay by 20ms to allow the physical button to go back up
    if (sample1 != 0) {
        vTaskDelay(pdMS_TO_TICKS(20)); // debounce 20ms
    }

    //then we sample again and place the bits into sample2
   
    for (int i = 0; i < 5; i++) {
        if (gpio_get_level(button_pins[i]) == 0) {
            //then we need to shift the data packet according to the index
            sample2 = sample2 | (1 << (i)); //its i because we need the 0th index, 
            //i+1 would be if something is occupying bit 0      
        }
    }
    //we need to do this to actually get a complete and clean button press.
    //eg if sample1 = 0000 0010
    // sample2 = 0000, 0001, then there must be a mistake and data_packet will be zero
    
    //but if
    //sample1 = 0000 0010
    //sample2 = 0000 0010 then 
    //data_packet = 0000 0010 and we return this.

    //so our data packet contains info of the buttons
    for (int i = 0; i<5; i++){
        if ((data_packet & (1<<i)) == 1) {
            //turn on the led
            gpio_set_level(led_pins[i], 1);
        }
        else {
            //turn off led
            gpio_set_level(led_pins[i], 0);
        }
    }

    //return the data packet.
    data_packet = sample1 & sample2; 
    return data_packet;
}


//making comments
//if bit 0 gets set then its pin 10

//0000 0001 - pin 10
//0000 0010 - pin 11
//0000 0100 - pin 12
//0000 1000 - pin 13
//0001 0000 - pin 14

//if we click two buttons at the same time like pin 10 and pin 11
//0000 0011 and both leds should turn on. need to write the code for the receiver too.

esp_err_t transmit_data(uint8_t *receiver_mac, uint8_t data) {
    esp_err_t result = esp_now_send(receiver_mac, &data, sizeof(data));
    return result;
}