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
#include "metrics.h"
static data_packet_t last_sent_packet = {0};
void deadband_filter(data_packet_t* packet, uint16_t raw_x, uint16_t raw_y) {
 // Deadband filter
        if (abs((int)raw_x - (int)last_sent_packet.joystick_x) < 25) {
            packet->joystick_x = last_sent_packet.joystick_x;
        } 
        else {
            packet->joystick_x = raw_x;
        }
        if (abs((int)raw_y - (int)last_sent_packet.joystick_y) < 25) {
            packet->joystick_y = last_sent_packet.joystick_y;
        } 
        else {
            packet->joystick_y = raw_y;
        }
}
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

    static uint32_t last_time = 0;
    //static uint8_t current_speed = 0;

    //eventually we will get rid of this super loop with preemptive scheduling 
    while (1)
    {
        metrics_record_loop_start();

        // Check for serial console commands
        UART_CONTROL_update();


        // Read joystick and buttons

        //create a clean zeroed out packet strcuture for this 10ms time frame.
        data_packet_t packet = {0};

        //print joystick debug readings to the console, not technically needed.
        print_joystick_values();


        //read the buttons first, and we sample the 5 push buttons with the debounce algo 
        packet.button_data = read_buttons();

        //call the new arrow and mode processor, then from reading the buttons we know how to process 
        //the arrow keys
        process_arrow_keys(&packet);

        //read the analog adc voltafes from the joystick.
        uint16_t raw_x = read_joystick_horizontal();
        uint16_t raw_y = read_joystick_vertical();
        


        //deadband filter.
        deadband_filter(&packet, raw_x, raw_y);

        
        packet.speed = current_speed; //after process joystick data we place the current speed into the packet.
        packet.mode = active_mode; //fill the mode into the packet.
        speaker_update(packet.button_data);

        // Transmit if data changed 
        if (memcmp(&packet, &last_sent_packet, sizeof(data_packet_t)) != 0) {
            
            //sent the 12 byte data packet to the robot over the 2.4GHz ESP-NOW
            last_sent_packet = packet;
            metrics_record_espnow_tx_start();
            transmit_data(receiver_mac, &packet);

            //track the time stamp of when we last sent a packet.
            uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());
            if (now - last_time > 250) {
                last_time = now;
            }
        }
        
        metrics_record_loop_end();

        //freertos non blocking delay, puts this task to sleep for 10ms, 
        //lets the cpu tackle other tasks 
        vTaskDelay(pdMS_TO_TICKS(10));  
    }
}