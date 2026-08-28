// receive data, and depending on what the action is we do something.
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

static const char *TAG = "ESP_RECEIVER";

int led_pins[] = {
    GPIO_NUM_10, // 0 (left) - manual mode
    GPIO_NUM_11, // idx 1 (down) - decrease speed
    GPIO_NUM_12, // idx 2 (up) - increase speed
    GPIO_NUM_13, // idx 3 (Stop) - stop
    GPIO_NUM_14  // idx 4 (right) - autonomous mode
};

void init_pins() {
    for (int i = 0; i < 5; i++) {
        gpio_reset_pin(led_pins[i]);
        gpio_set_direction(led_pins[i], GPIO_MODE_OUTPUT);
        gpio_set_level(led_pins[i], 0);
    }
}

void receive_button_press(data_packet_t* packet) {
    ESP_LOGI(TAG, "JoyX: %u | JoyY: %u | Speed: %u | Mode: %u", 
             packet->joystick_x, packet->joystick_y, packet->speed, packet->mode);

    for (int i = 0; i < 5; i++) {
        if ((packet->button_data & (1<<i)) != 0) {
            switch(i) {
                case LEFT_BTN:
                    gpio_set_level(led_pins[LEFT_BTN], 1);
                    ESP_LOGI(TAG, "BUTTON PRESSED -> MANUAL");
                    break;

                case RIGHT_BTN:
                    gpio_set_level(led_pins[RIGHT_BTN], 1);
                    ESP_LOGI(TAG, "BUTTON PRESSED -> AUTONOMOUS");
                    break;

                case UP_BTN:
                    ESP_LOGI(TAG, "Speed: %u \n", packet->speed);
                    gpio_set_level(led_pins[UP_BTN], 1);
                    break;

                case DOWN_BTN:
                    ESP_LOGI(TAG, "Speed: %u \n", packet->speed);
                    gpio_set_level(led_pins[DOWN_BTN], 1);
                    break;

                case STOP_BTN:
                    gpio_set_level(led_pins[STOP_BTN], 1);
                    ESP_LOGI(TAG, "STOP");
                    break;
            }
        }
        else {
            gpio_set_level(led_pins[i], 0);
        }
    }
}
