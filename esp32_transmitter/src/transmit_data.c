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

static const char *TAG = "TRANSMIT_DATA";

// Array for the GPIO pins to loop through and read
int button_pins[5] = {
    GPIO_NUM_10, // idx 0
    GPIO_NUM_11, // idx 1
    GPIO_NUM_12, // idx 2
    GPIO_NUM_13, // idx 3
    GPIO_NUM_14  // idx 4
};

typedef struct {
    uint8_t button_data;
    uint8_t fpga_data;
    uint8_t speed;
    uint16_t sequence;
} data_packet_t;

void init_button_pin(void) {
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_1, GPIO_PULLUP_ONLY);

    //we need to initialize all the pins to input
    for (int i = 0; i < 5; i++) {
        gpio_set_direction(button_pins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(button_pins[i], GPIO_PULLUP_ONLY);
    }

}

uint8_t read_buttons(void) {
    uint8_t data_packet = 0b00000000;
 
    // Active-low button: 0 when pressed to GND, 1 when released (pull-up)
    int button_press = gpio_get_level(GPIO_NUM_1);

    //button press is for the random button in the middle of the board
    if (button_press == 0) {
        vTaskDelay(pdMS_TO_TICKS(20)); // debounce 20ms
        if (gpio_get_level(GPIO_NUM_1) == 0) {
            data_packet |= (1 << 0);
            //set_led(true);  // Turn LED ON when button is pressed
        }
    }

    //this is for the buttons for the controller
    for (int i = 0; i < 5; i++) {
        if (gpio_get_level(button_pins[i]) == 0) {
            vTaskDelay(pdMS_TO_TICKS(20)); // debounce 20ms
            //then we need to shift the data packet according to the index
            data_packet = data_packet | (1 << (i)); //its i because we need the 0th index, 
            //i+1 would be if something is occupying bit 0 
        }
    }

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