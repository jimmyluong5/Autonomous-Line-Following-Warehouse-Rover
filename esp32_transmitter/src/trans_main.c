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
    // 1. Initialize Servo on Pin 14
    servo_init();

    // 2. Initialize Interactive UART / Serial Controller
    UART_CONTROL_init();

    // 3. Initialize peripherals & ESP-NOW
    init_led();
    init_esp_nvs();
    init_wifi();
    init_esp_now();
    init_button_pin();

    while (1)
    {
        // Interactive UART / Serial Monitor command processor
        UART_CONTROL_update();

        // 50Hz Servo PWM angle stepping
        Servo_Update();

        // ESP-NOW button transmission
        uint8_t packet = read_buttons();
        if (packet != 0)
        {
            transmit_data(receiver_mac, packet);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}