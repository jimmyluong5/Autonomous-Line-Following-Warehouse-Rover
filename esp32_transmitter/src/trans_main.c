#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "servo.h"
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


static const char *TAG = "MAIN";
void app_main(void)
{
    // 1. Peripherals, NVS, WiFi & ESP-NOW initialization
    init_led();
    init_esp_nvs();
    init_wifi();
    init_esp_now();
    init_button_pin();
    init_led_pin();
    init_joystick();
    init_speaker();
   
    
    

#if ENABLE_SERVO_MODE
    // 2. Servo & UART control initialization (if ENABLE_SERVO_MODE == 1)
    servo_init();
    UART_CONTROL_init();

    printf("\r\n==========================================\r\n");
    printf("   ESP32 SERVO CONTROLLER (%d°-%d° SAFE) \r\n", SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    printf("   Servo Pin: GPIO 14 (50Hz PWM)          \r\n");
    printf("==========================================\r\n");

    uint8_t sweep_angles[] = {
        SERVO_MIN_ANGLE,
        SERVO_MIN_ANGLE + (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE) / 4,
        SERVO_CENTER_ANGLE,
        SERVO_MAX_ANGLE - (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE) / 4,
        SERVO_MAX_ANGLE,
        SERVO_CENTER_ANGLE
    };
    uint8_t sweep_idx = 0;
    uint32_t last_sweep_time = 0;
#else
    printf("\r\n==========================================\r\n");
    printf("   ESP32 TRANSMITTER (SERVO MODE DISABLED) \r\n");
    printf("   Pure ESP-NOW Button Transmission Ready \r\n");
    printf("==========================================\r\n");
#endif

    static data_packet_t last_sent_packet = {0};
    static uint32_t last_time = 0;
    while (1)
    {
#if ENABLE_SERVO_MODE
        // Check for manual interactive commands from Serial Monitor
        UART_CONTROL_update();

        // Continuous 50Hz PWM update
        Servo_Update();

        // Safe auto sweep when manual mode is not active
        if (!UART_CONTROL_IsManualActive())
        {
            uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());
            if (now - last_sweep_time >= 2000)
            {
                last_sweep_time = now;
                uint8_t next_angle = sweep_angles[sweep_idx];
                sweep_idx = (sweep_idx + 1) % 6;

                Servo_SetAngle(next_angle);
                printf("[SAFE SWEEP] Target: %d deg | Current: %d deg\r\n",
                       next_angle, Servo_GetCurrentAngle());
            }
        }
#endif

        data_packet_t packet;


        print_joystick_values();
        // Read button and transmit state changes (0x01 on press, 0x00 on release) over ESP-NOW
        packet.button_data = read_buttons();
        uint16_t raw_x = read_joystick_horizontal();
        uint16_t raw_y= read_joystick_vertical(); //reads the pins
        packet.speed = 100; //initialize the packet speed.
        

        //implement deadband where < X counts compared to last packets 
        if (abs( (int)raw_x - (int)last_sent_packet.joystick_x) < 25 ) {
            //set the packet.joystick_x to last_sent packet
            packet.joystick_x = last_sent_packet.joystick_x;
        }
        else {
            //set the packet of joystick x to the raw value.
            packet.joystick_x = raw_x;
        }

        if (abs( (int)raw_y - (int)last_sent_packet.joystick_y < 25 )) {
            //set the packet.joystick_x to last_sent packet
            packet.joystick_y = last_sent_packet.joystick_y;
        }

        else {
            //set the packet of joystick x to the raw value.
            packet.joystick_y = raw_y;
        }

        speaker_update(packet.button_data);

        //check if any data in the packet changed using memcpy()
        if ((memcmp(&packet, &last_sent_packet, sizeof(data_packet_t))) !=0) {
            //update the last sent packet
            last_sent_packet = packet;

            //transmit the address of the struct over esp-now so the receiver can 
            //access it 
            transmit_data(receiver_mac, &packet);

            //need to slow down putty sending messages
            uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());

            if (now - last_time > 250) { //if the difference between these times is 250ms
                //set the last time to current time
                last_time = now;

                //print the messages
                ESP_LOGI(TAG, "[ESP-NOW] Sent packet - Buttons: 0x%02X | Speed: %d | JoyX: %u | JoyY: %u\r\n",
                packet.button_data, packet.speed, packet.joystick_x, packet.joystick_y);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));  
    }
}