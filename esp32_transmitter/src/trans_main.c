#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "servo.h"
#include "setup.h"
#include "transmit_data.h"
#include "uart_control.h"
#include <stdio.h>

void app_main(void)
{
    // 1. Initialize hardware
    servo_init();
    UART_CONTROL_init();
    init_led();
    init_esp_nvs();
    init_wifi();
    init_esp_now();
    init_button_pin();

    printf("\r\n==========================================\r\n");
    printf("   ESP32 SERVO CONTROLLER (%d°-%d° SAFE) \r\n", SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
    printf("   Servo Pin: GPIO 14 (50Hz PWM)          \r\n");
    printf("==========================================\r\n");

    // Dynamic sweep angles calculated directly from servo.h limits
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

    while (1)
    {
        // 1. Check for manual interactive commands from Serial Monitor
        UART_CONTROL_update();

        // 2. Continuous 50Hz PWM update
        Servo_Update();

        // 3. If not in manual mode or stopped, run auto sweep
        if (!UART_CONTROL_IsManualActive())
        {
            uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount());
            if (now - last_sweep_time >= 2000)
            {
                last_sweep_time = now;
                uint8_t next_angle = sweep_angles[sweep_idx];
                sweep_idx = (sweep_idx + 1) % 6;

                Servo_SetAngle(next_angle);
                blink_led();
                printf("[SAFE SWEEP] Target: %d deg | Current: %d deg\r\n",
                       next_angle, Servo_GetCurrentAngle());
            }
        }

        // 4. Read button and transmit over ESP-NOW
        uint8_t packet = read_buttons();
        if (packet != 0)
        {
            transmit_data(receiver_mac, packet);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}