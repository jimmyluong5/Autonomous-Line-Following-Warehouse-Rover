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

typedef struct {
    uint8_t button_data;
    uint8_t fpga_data;
    uint8_t speed;
    uint16_t sequence;
} data_packet;

void init_button_pin(void) {
    gpio_set_direction(GPIO_NUM_1, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_1, GPIO_PULLUP_ONLY);
}

uint8_t read_buttons(void) {
    uint8_t data_packet = 0b00000000;

    // Active-low button: 0 when pressed to GND, 1 when released (pull-up)
    int button_press = gpio_get_level(GPIO_NUM_1);

    if (button_press == 0) {
        vTaskDelay(pdMS_TO_TICKS(20)); // debounce 20ms
        if (gpio_get_level(GPIO_NUM_1) == 0) {
            data_packet |= (1 << 0);
            //set_led(true);  // Turn LED ON when button is pressed
        }
    } else {
        //set_led(false);     // Turn LED OFF when button is released
    }

    return data_packet;
}

esp_err_t transmit_data(uint8_t *receiver_mac, uint8_t data) {
    esp_err_t result = esp_now_send(receiver_mac, &data, sizeof(data));
    return result;
}