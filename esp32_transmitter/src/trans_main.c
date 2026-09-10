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
#include "stdbool.h"
#include "esp_mac.h"

#define failsafe_time 2000
bool failsafe_flag = false;

static data_packet_t last_sent_packet = {0};
uint32_t last_time_rx = 0;
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

void check_failsafe(data_packet_t *packet) {
    //if we're in the manual page
    if (current_page == PAGE_MANUAL || current_page == PAGE_MANUAL_DATA) {
        //if the time is greater than 2000ms then we return to the menu 
        uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());
        
        //then we check if we pressed a button or the joystick moved
        bool user_active = (packet->button_data !=0 || (abs(metrics_get_joy_x_val()) > 10) || (abs(metrics_get_joy_y_val()) > 10)); 
        
        //if active then we set the last time to now,
        if (user_active == true) {
            last_time_rx = now; //so we reset the time if the user pressed the button or
        }

         // 2. If 5 seconds of idle inactivity passed, trip!
        if ((now - last_time_rx) > 5000) {
            failsafe_flag = true; 
            last_time_rx = now;
            speaker_pattern(8, 75, 75);
            ESP_LOGW("FAILSAFE", "Inactivity timeout (5s), activating failsafe!");
            // Return to menu
            current_page = PAGE_MENU;
            active_mode = MENU_MODE;
            hovered_mode = MANUAL_MODE;
        }
        
        
    }
}
void app_main(void) {
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

        
        packet.speed = (current_speed > 0) ? current_speed : 128; // default to 50% speed
        packet.mode = active_mode; //fill the mode into the packet.
        speaker_update(packet.button_data);

        // Transmit continuously at 40 Hz (every 25ms) or immediately if data changed
        uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount()); //this is the current time
        if (memcmp(&packet, &last_sent_packet, sizeof(data_packet_t)) != 0 || (now - last_time >= 25)) {
            last_sent_packet = packet;
            last_time = now;
            
        
            metrics_record_espnow_tx_start();
            transmit_data(receiver_mac, &packet);
        }
        //check failsafe every iteration
        check_failsafe(&packet);

        // Print transmitter MAC address every 3 seconds to console
        static uint32_t last_mac_print = 0;
        if (now - last_mac_print > 3000) {
            last_mac_print = now;
            uint8_t mac[6];
            esp_read_mac(mac, ESP_MAC_WIFI_STA);
            printf(">> TRANSMITTER MAC: {0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X}\r\n",
                   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        }
       
        
        metrics_record_loop_end();

        //freertos non blocking delay, puts this task to sleep for 10ms, 
        //lets the cpu tackle other tasks 
        vTaskDelay(pdMS_TO_TICKS(10));  
    }
}