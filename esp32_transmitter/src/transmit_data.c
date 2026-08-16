// accept an input data from a button click, but that button is pressed on the
// esp32 send it via ESP-NOW, the esp32 receives the data, it will then send it
// to the LED to blink it.

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
//need to make the data packet struct

static const char *TAG = "TRANSMIT_DATA";

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

    //add a pull up resistor so pin stays high until pressed to gnd
    gpio_set_pull_mode(GPIO_NUM_1, GPIO_PULLUP_ONLY);
}

uint8_t read_buttons(void) {
    uint8_t data_packet = 0b00000000;

    //read the initial button state
    int button_press = gpio_get_level(GPIO_NUM_1);

    //if pin is 0 (active-low button pressed), debounce with a 20ms delay
    if (button_press == 0) {
        vTaskDelay(pdMS_TO_TICKS(20)); // wait 20ms for mechanical bouncing to settle
        if (gpio_get_level(GPIO_NUM_1) == 0) { // confirm it is still pressed
            data_packet |= (1 << 0); // set LSB to 1
        }
    }
    return data_packet;
}

//need to send the data packet via esp-now
esp_err_t transmit_data(uint8_t *receiver_mac, uint8_t data) {
    //to transmit data, just send the data packet into 
    //esp_now_send()
    
    //we need to pass the address of data into esp_now_send

    //check the return value of esp_now_send() to see if it was successful.
    //it would be a esp_err_t
    
    //esp_err_t is the standard error status type used in every esp32 function.

    

    esp_err_t result;
    //so we send the receiver_mac into this function, 
    // the memory address of the data, and the size of the data.
    result = esp_now_send(receiver_mac, &data, sizeof(data));
    //print the result if it fails
    if (result != ESP_OK) { //esp_ok is == 0 which means success, any other number then fail.
        ESP_LOGE(TAG, "Failed to send data");
    }   
    return result; //returns if the data could be sent or not. we can use the OnDataSent callback

    //the OnDataSent callback is called when the data is sent.
    //which is a function that prints a message to the console.
    //it gives us feedback on whether the data was sent successfully or not.


}