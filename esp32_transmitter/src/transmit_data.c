#include <stdio.h>
#include <stdlib.h>
#include <esp_now.h>
#include <stdint.h>
#include <string.h>
#include <driver/gpio.h>
#include "esp_log.h"
#include "setup.h"
#include "transmit_data.h"
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

#define BTN_LEFT   (1 << 0) // GPIO 10: Left Arrow
#define BTN_DOWN   (1 << 1) // GPIO 11: Down Arrow
#define BTN_UP     (1 << 2) // GPIO 12: Up Arrow
#define BTN_CENTER (1 << 3) // GPIO 13: Center / Confirm
#define BTN_RIGHT  (1 << 4) // GPIO 14: Right Arrow


static const char *TAG = "TRANSMIT_DATA";
// mode_selections array definition

page_t current_page = PAGE_MENU;
int hovered_mode = 0;
uint8_t active_mode = MANUAL_MODE; //default mode is the manual mode sure

int page_length = PAGE_MAX_COUNT -1;
int mode_length = TOTAL_MODES-1;

// Array for the GPIO pins to loop through and read

int button_pins[] = {
    GPIO_NUM_10, //0 (left) //manual mode
    GPIO_NUM_11, //idx 1 (down) //decrease speed by 10 % (0-255 then its by 25 counts or 5% = 13 counts)
    GPIO_NUM_12, //idx 2 (up) //increase speed by 10% or 5%
    GPIO_NUM_13, //idx 3 (Stop) //just stop
    GPIO_NUM_14 //idx 4 (right) //autonomous mode.
};

int modes_selection[] = {
    LEFT_BTN,   // Index 0: MANUAL_MODE (Left Button = 0)
    RIGHT_BTN,  // Index 1: AUTO_MODE (Right Button = 4)
    IMU_MODE,   // Index 2: IMU_MODE (Placeholder)
};


void process_arrow_keys(data_packet_t *packet) {
    //last button press
    static uint8_t last_button_state = 0;

    //detect new button press using riding edge
    uint8_t just_pressed = packet->button_data & (~last_button_state);
    last_button_state = packet->button_data;

    //if we didn't press the button then just return
    if (just_pressed == 0) {
        return;
    }

    //right arrow 
    //you logical and it because if they both have a set bit in the lsb,
    //then its a right button click.
    if (just_pressed & BTN_RIGHT) {
        if (current_page < page_length) { // 4 pages total (0 to 3)
            // PAGE_MENU     = 0 (Mode selection + Rover animation)
            // PAGE_GITHUB   = 1 (GitHub QR)
            // PAGE_LINKEDIN = 2 (LinkedIn QR)
            // PAGE_LEFTPAGE = 3 (Left page - left.jpg)
            
            //increase the current page by 1, because we're going to the next page.
            current_page++;
            //print on putty, not really needed
            ESP_LOGI(TAG, "Next Page -> Page, %d", current_page);
        }
        return; //handled the page turn
    }

    //left arrow
    if (just_pressed & BTN_LEFT) {
        if (current_page > 0) {
            current_page--; //decrease cuz we going left.
            //print on putty
            ESP_LOGI(TAG, "Next Page -> %d", current_page);
        }
        return;
    }

    //if we are in the menu page.
    if (current_page == PAGE_MENU) {
        
        //up arrow
        if (just_pressed & BTN_UP) {
            hovered_mode--;
            if (hovered_mode < 0)  {
                hovered_mode = mode_length; // wrap around 
            }
            ESP_LOGI(TAG, "Cursor UP -> Mode %d", hovered_mode);
        }

        //down arrow
        if (just_pressed & BTN_DOWN) {
            hovered_mode++;
             if (hovered_mode >= TOTAL_MODES)  {
                hovered_mode = 0;   // wrap around
            }
            ESP_LOGI(TAG, "Cursor DOWN -> Mode %d", hovered_mode);
        }

        //center button, we clicked the center button.
        if (just_pressed & BTN_CENTER) {
            active_mode = hovered_mode; //set the active mode to whatever hovered_mode we're in.
            //then put the next mode into active_mode
            packet->mode = active_mode; //update esp_now packet, 
            //so we can send it to the rover.
            ESP_LOGI(TAG, "*** ACTIVE MODE CONFIRMED: %d ***", active_mode);


            //just change the code here, like adding modes, so if
            //we are in the manual mode then change the active mode to the manual mode, 
            switch (hovered_mode) {
                case MANUAL_MODE:
                    current_page = PAGE_MANUAL;
                    break;
                case AUTO_MODE:
                    current_page = PAGE_AUTO;
                    break;
                case IMU_MODE: 
                    current_page = PAGE_IMU;
                    break;
                break; //need default state here?
            }
            
            switch(current_page) {
                case PAGE_MANUAL:
                    //we are in the manual page, now we can check if we click the left btn
                    //then just move back to the menu
                    if (just_pressed & BTN_LEFT) {
                        current_page = PAGE_MENU;
                        ESP_LOGI(TAG, "Back to Menu Page");
                    }
                    else {
                        //we move to the 2nd page.
                        current_page = PAGE_MANUAL_DATA;
                        //we can display in putty
                        ESP_LOGI(TAG, "Manual Page Data");
                    }
                    break;

                case PAGE_AUTO:
                    if (just_pressed & BTN_LEFT) {
                        current_page = PAGE_MENU;
                        ESP_LOGI(TAG, "Back to Menu Page");
                    }
                    else {
                        current_page = PAGE_AUTO_DATA;
                        ESP_LOGI(TAG, "Auto Page Data");
                    }
                    break;
                case PAGE_IMU:
                    if (just_pressed & BTN_LEFT) {
                            current_page = PAGE_MENU;
                            ESP_
                            LOGI(TAG, "Back to Menu Page");
                    }
                    else {
                        current_page = PAGE_IMU_DATA;
                        ESP_LOGI(TAG, "IMU Page Data");
                    }
                    break;
            }

            //then we check if the current page is page manual or auto or imu

           
            
        }

    }
}



void init_button_pin(void) {

    //we need to initialize all the pins to input
    for (int i = 0; i < 5; i++) {
        gpio_set_direction(button_pins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(button_pins[i], GPIO_PULLUP_ONLY);
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
    //so our data packet contains info of the buttons

    static uint8_t last_buttons = 0x00;
    uint8_t new_presses = data_packet & (~last_buttons);
    last_buttons = data_packet;

    for (int i = 0; i < 5; i++) {
        if ((data_packet & (1 << i)) != 0) {
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