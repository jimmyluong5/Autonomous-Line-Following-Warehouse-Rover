#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "setup.h"
#include "transmit_data.h"
#include "uart_control.h"
#include <stdio.h>
#include "adc.h"
#include "joystick.h"
#include "speaker.h"
#include "stdint.h"
#include <string.h>
#include "esp_log.h"
#include "math.h"
#include "lcd.h"

void app_main(void)
{
    // 1. Peripherals, NVS, WiFi, LCD, UART & ESP-NOW initialization
    init_esp_nvs();
    init_wifi();
    init_esp_now();
    init_button_pin();
    init_joystick();
    init_speaker();
    init_lcd_driver();
    UART_CONTROL_init();

    printf("\r\n==========================================\r\n");
    printf("   ESP32 TRANSMITTER READY               \r\n");
    printf("   Pure ESP-NOW Button Transmission Ready \r\n");
    printf("==========================================\r\n");

    static data_packet_t last_sent_packet = {0};
    static uint32_t last_time = 0;
    static uint8_t current_speed = 0;
    static uint8_t current_mode = MANUAL_MODE;

    while (1)
    {
        // Check for serial console commands
        UART_CONTROL_update();

        // Read joystick and buttons
        data_packet_t packet = {0};
        print_joystick_values();

        packet.button_data = read_buttons();
        uint16_t raw_x = read_joystick_horizontal();
        uint16_t raw_y = read_joystick_vertical();
        
        packet.speed = current_speed;
        packet.mode = current_mode;
        
        current_speed = update_speed(&packet);
        current_mode = update_mode(&packet);

        // Deadband filter
        if (abs((int)raw_x - (int)last_sent_packet.joystick_x) < 25) {
            packet.joystick_x = last_sent_packet.joystick_x;
        } else {
            packet.joystick_x = raw_x;
        }

        if (abs((int)raw_y - (int)last_sent_packet.joystick_y) < 25) {
            packet.joystick_y = last_sent_packet.joystick_y;
        } else {
            packet.joystick_y = raw_y;
        }

        speaker_update(packet.button_data);

        // Transmit if data changed
        if (memcmp(&packet, &last_sent_packet, sizeof(data_packet_t)) != 0) {
            last_sent_packet = packet;
            transmit_data(receiver_mac, &packet);

            uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());
            if (now - last_time > 250) {
                last_time = now;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));  
    }
}