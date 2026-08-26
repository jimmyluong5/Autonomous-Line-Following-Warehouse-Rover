// receive data, and depending on what the action is we do something.
//like the goal is to turn on an LED from the other esp32 button click. 
#include "receive_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <esp_now.h>
#include <stdint.h>
#include <string.h>
#include <driver/gpio.h>
#include "esp_log.h"

#define LED_PIN GPIO_NUM_1

#define LEFT_BTN 0
#define DOWN_BTN 1
#define UP_BTN 2
#define STOP_BTN 3
#define RIGHT_BTN 4

//its just a label we use that appears in the log when we do messages
//"ESP_RECEIEVER: LEFT IS PRESSED"
static const char *TAG = "ESP_RECEIVER";

//make global variable speed (8bit)
uint8_t speed = 0;
//make array for the button pins

int button_pins[] = {
    GPIO_NUM_10, //0 (left) //manual mode
    GPIO_NUM_11, //idx 1 (down) //decrease speed by 10 % (0-255 then its by 25 counts or 5% = 13 counts)
    GPIO_NUM_12, //idx 2 (up) //increase speed by 10% or 5%
    GPIO_NUM_13, //idx 3 (Stop) //just stop
    GPIO_NUM_14 //idx 4 (right) //autonomous mode.
};


typedef struct{
    uint8_t button_data;
    uint8_t speed; //(0-255 8 bit integer.)
    uint16_t sequence;
} data_packet;

//need to initialize pin

void init_pins() {
    for (int i = 0; i < 5; i++) {
        gpio_set_direction(button_pins[i], GPIO_MODE_OUTPUT);

        //then set all of the off at the beginning
        gpio_set_level(button_pins[i], 0);
    }
}




//static uint8_t s_last_state = 0xFF; / track previous button state

//we receive input data in the form of the data packet, then output a 1 or 0 and give it to the LED
void receive_button_press(uint8_t data) {

    //so my data contains all that information
    //esp_logi inputs are 
    for (int i = 0; i < 5; i++) {
        //bitwise and to ensure that we have the correct button data, or 1s in the correct spot.
        if (data & (1<<i)) {
            //if the data has a 1 in it, then set the bit to turn on the led
            
            //use a switch statement to keep track of all of this
            switch(i) {
                //left_btn will be a number and you just left shift that bit into a 1, and if its actually a 1 there
                //then we have a valid press

                //because if data = 0000_0001 and 1<<0 turns into this 0000_0001 so & turns into bitwise and.
                case LEFT_BTN:
                    //turn the led on
                    gpio_set_level(button_pins[LEFT_BTN], 1);
                    ESP_LOGI(TAG, "MANUAL MODE");
                    break;

                case RIGHT_BTN:
                    gpio_set_level(button_pins[RIGHT_BTN], 1);
                    ESP_LOGI(TAG, "AUTONOMOUS MODE");
                    break;

                case UP_BTN:
                    update_speed(data);
                    ESP_LOGI(TAG, "Speed: %u \n", speed);
                    gpio_set_level(button_pins[UP_BTN], 1);
                    break;

                case DOWN_BTN:
                    update_speed(data);
                    ESP_LOGI(TAG, "Speed: %u \n", speed);
                    gpio_set_level(button_pins[DOWN_BTN], 1);

                    break;

                case STOP_BTN:
                    gpio_set_level(button_pins[STOP_BTN], 1);
                    ESP_LOGI(TAG, "STOP");
                    break;
            }
        }
        else {
            gpio_set_level(button_pins[i], 0);
        }
    }
}


void update_speed(uint8_t data){
    //we receive speed and analyze the bits, 

    //each time we see a set bit in the locations of the data packets where up and down are
    //then decrease by 5% or 13 counts
    //since we will receive valid data, we only need to look at the bits
    if (data & (1<<UP_BTN)) {
        if (speed > 255-13) {
            speed = 255;
            ESP_LOGI(TAG, "MAX SPEED ACHIEVED\n");
        }
        else {
            speed += 13;
            ESP_LOGI(TAG, "INCREASING SPEED BY 5%% \n");
        }
    }
    
    else if (data & (1<<DOWN_BTN)) {
        if (speed < 13){
            speed = 0;
            ESP_LOGI(TAG, "LOWEST SPEED ACHIEVED\n");
            gpio_set_level(button_pins[DOWN_BTN], 1);
        }
        else {
            speed -=13;
            ESP_LOGI(TAG, "DECREASING SPEED BY 5%% \n");
            gpio_set_level(button_pins[DOWN_BTN], 0);
        }

    }

    //we don't need an else if statement because the data we getting is guaranteed to be valid.
}
