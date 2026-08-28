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

    static uint8_t last_sent_packet = 0xFF;

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
        print_joystick_values();
        // Read button and transmit state changes (0x01 on press, 0x00 on release) over ESP-NOW
        uint8_t packet = read_buttons(); //reads the pins
        speaker_update(packet);
        if (packet != last_sent_packet) //if the packet changes, transmit it, like if the button state changes 

        {
            //so if packet is not equal to the last packet sent, then our button state has changed.
            last_sent_packet = packet; //so we update the last sent packet

            //then we send the next packet.
            transmit_data(receiver_mac, packet);
            //prints in putty/uart?
            //printf("[ESP-NOW] Transmitted packet: 0x%02X (Receiver LED %s)\r\n",packet, (packet & 0x01) ? "ON" : "OFF");
        }

        vTaskDelay(pdMS_TO_TICKS(10)); //this is like the debounce time 
    }
}