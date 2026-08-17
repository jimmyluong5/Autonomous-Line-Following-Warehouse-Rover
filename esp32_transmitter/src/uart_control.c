#include "uart_control.h"
#include "servo.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define UART_PORT_NUM UART_NUM_0
#define BUF_SIZE 512

static uint8_t current_servo_angle = 90;
static UART_ControlMode current_mode = UART_MODE_MENU;

static void UART_SendMessage(const char *message) {
    if (uart_is_driver_installed(UART_PORT_NUM)) {
        uart_write_bytes(UART_PORT_NUM, message, strlen(message));
    }
    printf("%s", message);
    fflush(stdout);
}

void UART_CONTROL_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    if (!uart_is_driver_installed(UART_PORT_NUM)) {
        uart_driver_install(UART_PORT_NUM, BUF_SIZE, 0, 0, NULL, 0);
        uart_param_config(UART_PORT_NUM, &uart_config);
    }

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    current_mode = UART_MODE_MENU;

    char menu_buf[512];
    snprintf(menu_buf, sizeof(menu_buf),
             "\r\n\x1b[2J\x1b[H"
             "==========================================\r\n"
             "       Warehouse Rover Control Menu\r\n"
             "==========================================\r\n"
             "Select Option/Mode:\r\n"
             " [s] - Servo Control Mode\r\n"
             "==========================================\r\n");
    UART_SendMessage(menu_buf);
}

void UART_CONTROL_update(void) {
    uint8_t received_byte = 0;
    int len = 0;

    // 1. Try reading from hardware UART driver (non-blocking, timeout 0)
    if (uart_is_driver_installed(UART_PORT_NUM)) {
        len = uart_read_bytes(UART_PORT_NUM, &received_byte, 1, 0);
    }

    // 2. Fallback to stdin non-blocking read
    if (len <= 0) {
        int c = fgetc(stdin);
        if (c != EOF && c > 0 && c < 128) {
            received_byte = (uint8_t)c;
            len = 1;
        }
    }

    if (len > 0) {
        if (current_mode == UART_MODE_MENU) {
            if (received_byte == 's' || received_byte == 'S') {
                current_mode = UART_MODE_SERVO;
                current_servo_angle = 90;
                Servo_SetAngle(current_servo_angle);
                UART_SendMessage("\r\n\x1b[2J\x1b[H"
                                 "--- Servo Control Mode Active ---\r\n"
                                 "Controls:\r\n"
                                 " [a] - Decrease angle by 5 deg\r\n"
                                 " [d] - Increase angle by 5 deg\r\n"
                                 " [1] - Set to 0 deg | [2] - Set to 45 deg | [3] - Set to 90 deg \r\n"
                                 " [4] - Set to 135 deg | [5] - Set to 180 deg\r\n"
                                 " [h] - Return to Main Menu\r\n"
                                 "---------------------------------\r\n"
                                 "Current Angle:  90 degrees");
            } else if (received_byte == 'h' || received_byte == 'H' || received_byte == '\r' || received_byte == '\n' || received_byte == ' ') {
                UART_CONTROL_init();
            }
        } else if (current_mode == UART_MODE_SERVO) {
            bool angle_changed = false;
            if (received_byte == 'h' || received_byte == 'H') {
                UART_SendMessage("\r\n--- Exited Servo Mode ---\r\n");
                UART_CONTROL_init();
            } else if (received_byte == 'a' || received_byte == 'A') {
                current_servo_angle = (current_servo_angle >= 5) ? current_servo_angle - 5 : 0;
                angle_changed = true;
            } else if (received_byte == 'd' || received_byte == 'D') {
                current_servo_angle = (current_servo_angle <= 175) ? current_servo_angle + 5 : 180;
                angle_changed = true;
            } else if (received_byte == '1') {
                current_servo_angle = 0;
                angle_changed = true;
            } else if (received_byte == '2') {
                current_servo_angle = 45;
                angle_changed = true;
            } else if (received_byte == '3') {
                current_servo_angle = 90;
                angle_changed = true;
            } else if (received_byte == '4') {
                current_servo_angle = 135;
                angle_changed = true;
            } else if (received_byte == '5') {
                current_servo_angle = 180;
                angle_changed = true;
            }

            if (angle_changed) {
                Servo_SetAngle(current_servo_angle);
                char angle_buf[64];
                snprintf(angle_buf, sizeof(angle_buf), "\rCurrent Angle: %3d degrees", current_servo_angle);
                UART_SendMessage(angle_buf);
            }
        }
    }
}

UART_ControlMode UART_CONTROL_GetMode(void) {
    return current_mode;
}
