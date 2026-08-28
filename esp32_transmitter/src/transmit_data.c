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
#include "speaker.h"

#define LEFT_BTN 0
#define DOWN_BTN 1
#define UP_BTN 2
#define STOP_BTN 3
#define RIGHT_BTN 4

#define MANUAL_MODE 0
#define AUTO_MODE 1
#define IMU_MODE 2
#define PHONE_MODE 3

static const char *TAG = "TRANSMIT_DATA";
// mode_selections array definition



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

int modes_selection[] = {
    LEFT_BTN,   // Index 0: MANUAL_MODE (Left Button = 0)
    RIGHT_BTN,  // Index 1: AUTO_MODE (Right Button = 4)
    IMU_MODE,   // Index 2: IMU_MODE (Placeholder)
    PHONE_MODE, // Index 3: PHONE_MODE (Placeholder)
};

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

            //make the speaker sound.
        }
    }
    //we need to do this to actually get a complete and clean button press.
    //eg if sample1 = 0000 0010
    // sample2 = 0000, 0001, then there must be a mistake and data_packet will be zero
    
    //but if
    //sample1 = 0000 0010
    //sample2 = 0000 0010 then 
    //data_packet = 0000 0010 and we return this.

   

    
    data_packet = sample1 & sample2; 
    //this is the code for the led button clicking.
    //so our data packet contains info of the buttons

    static uint8_t last_buttons = 0x00;
    uint8_t new_presses = data_packet & (~last_buttons);
    last_buttons = data_packet;

    for (int i = 0; i < 5; i++) {
        if ((data_packet & (1 << i)) != 0) {
            // Keep LED ON while button is physically pressed
            gpio_set_level(led_pins[i], 1);

            // Log Putty message ONLY ONCE per click on new press
            if ((new_presses & (1 << i)) != 0) {
                switch(i) {
                    case LEFT_BTN:
                        ESP_LOGI(TAG, "MANUAL MODE");
                        break;
                    case RIGHT_BTN:
                        ESP_LOGI(TAG, "AUTONOMOUS MODE");
                        break;
                    case STOP_BTN:
                        //ESP_LOGI(TAG, "STOP | Speed = %u", packet->speed);
                        break;
                    case UP_BTN:
                    case DOWN_BTN:
                        break;
                }
            }
        }
        else {
            // Turn OFF LED when button is released
            gpio_set_level(led_pins[i], 0);
        }
    }
    //return the data packet.
    return data_packet;
}
uint8_t update_speed(data_packet_t *packet){
    static uint8_t last_button_state = 0x00;
    static uint32_t last_speed_trigger = 0;

    //we need to detect newly pressed buttons
    //detects on rising edge.
    uint8_t new_presses = packet->button_data & (~last_button_state);
    last_button_state = packet->button_data;

    uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());

    //debounce lockout: ignore fast contact bounce triggers within 150ms of last press
    if (new_presses != 0 && (now - last_speed_trigger < 150)) {
        return packet->speed;
    }

    //we receive the data packet and need to extract the data. 
    
    //each time we see a set bit in the locations of the data packets where up and down are
    //then decrease by 5% or 13 counts
    //since we will receive valid data, we only need to look at the bits
    for (int i = 0; i < 5; i++) {
        if (new_presses & (1 << i)) {
            last_speed_trigger = now;
            switch(i) {
                case STOP_BTN:
                    packet->speed = 0;
                    ESP_LOGI(TAG, "STOP | Speed = %u\n", packet->speed);
                    break;

                case UP_BTN: //packet->button_data we access
                    //the button data and check if its valid data.
                    if (packet->speed > 255-13) { //we access speed using packet->speed.
                        packet->speed = 255;
                        ESP_LOGI(TAG, "MAX SPEED ACHIEVED (%u)\n", packet->speed);
                    }

                    else {
                        packet->speed += 13;
                        ESP_LOGI(TAG, "INCREASING SPEED BY 5%% ->Speed: %u\n", packet->speed);
                    }
                    break;

                case DOWN_BTN:
                    if (packet->speed < 13){
                        packet->speed = 0;
                        ESP_LOGI(TAG, "LOWEST SPEED ACHIEVED (%u)\n", packet->speed);
                        //gpio_set_level(button_pins[DOWN_BTN], 1);
                        //not needed because we already do it in the loop above/
                    }

                    else {
                        packet->speed -= 13;
                        ESP_LOGI(TAG, "DECREASING SPEED BY 5%% ->Speed: %u\n", packet->speed);
                    }
                    break;
            }
        }
    }

    //we don't need an else if statement because the data we getting 
    // is guaranteed to be valid.
    return packet->speed;
}

uint8_t update_mode(data_packet_t *packet) {
    static uint8_t current_mode = MANUAL_MODE; // default mode
    static uint8_t last_button_state = 0x00;   // debouncing / rising edge detection
    static uint32_t last_mode_trigger = 0;

    uint8_t new_presses = packet->button_data & (~last_button_state); // register on rising edge
    last_button_state = packet->button_data;

    uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());

    //debounce lockout: ignore contact bounce triggers within 150ms
    if (new_presses != 0 && (now - last_mode_trigger < 150)) {
        packet->mode = current_mode;
        return current_mode;
    }

    // Loop through assigned mode buttons (i=0: MANUAL_MODE/LEFT_BTN, i=1: AUTO_MODE/RIGHT_BTN)
    for (int i = 0; i < 2; i++) {
        if (new_presses & (1 << modes_selection[i])) {
            last_mode_trigger = now;
            switch (i) {
                case MANUAL_MODE:
                    current_mode = MANUAL_MODE;
                    ESP_LOGI(TAG, "Mode Updated -> MANUAL MODE (%u)\n", current_mode);
                    break;

                case AUTO_MODE:
                    current_mode = AUTO_MODE;
                    ESP_LOGI(TAG, "Mode Updated -> AUTONOMOUS MODE (%u)\n", current_mode);
                    break;
                /*
                case IMU_MODE:
                    current_mode = IMU_MODE;
                    ESP_LOGI(TAG, "Mode Updated -> IMU MODE (%u)\n", current_mode);
                    break;
                
                case PHONE_MODE:
                    current_mode = PHONE_MODE;
                    ESP_LOGI(TAG, "Mode Updated -> PHONE MODE (%u)\n", current_mode);
                    break;
                */
                
            }
        }
    }
    
    packet->mode = current_mode;
    return current_mode;
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

//instead of sending just 8 bit data, it's gonna be an entire data packet.
esp_err_t transmit_data(const uint8_t *receiver_mac, const data_packet_t *packet) {
    esp_err_t result = esp_now_send(receiver_mac, (const uint8_t*) packet, sizeof(data_packet_t));
    return result;
}