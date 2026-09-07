// receive data, and depending on what the action is we do something.
#include "receive_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <esp_now.h>
#include <stdint.h>
#include <string.h>
#include <driver/gpio.h>
#include "esp_log.h"
#include "driver/uart.h"
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

    //we can just send the data packet to the stm32
    send_packet_stm32(packet);
    
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

void send_packet_stm32(data_packet_t *packet) {
    //create a 1 byte marker where if we see this marker then we know its the correct data packet
    uint8_t marker = 0xAA;

    //then just send the datapacket and the marker byte via uart, we are using port 1
    uart_write_bytes(UART_NUM_1, (const char*)&marker, 1);

    //send the actual data packet
    uart_write_bytes(UART_NUM_1, packet, sizeof(data_packet_t));
    //don't need to typecast the packet to (const char*), just send the packet which contains the address of the struct.
}

//uart_write_bytes inputs, is the uart port, the address of the variable we're sending,
//third is the size of the packet.

//uart doesn't know what a struct is, but it does lnow what are bytes.