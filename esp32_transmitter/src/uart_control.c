#include "uart_control.h"
#include "servo.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define UART_PORT UART_NUM_0
#define BUF_SIZE 512

static uint8_t current_servo_angle = 90;
static UART_ControlMode current_mode = UART_MODE_SERVO;

static void UART_SendMessage(const char *message) {
    if (uart_is_driver_installed(UART_PORT)) {
        uart_write_bytes(UART_PORT, message, strlen(message));
    }
    printf("%s", message);
    fflush(stdout);
}

void print_main_menu(void) {
    UART_SendMessage(
        "\r\n==========================================\r\n"
        "     ESP32 SERVO INTERACTIVE CONTROLLER   \r\n"
        "==========================================\r\n"
        "Controls (Type key & press Enter):\r\n"
        " [a] / [d]  : Decrease / Increase angle by 5 deg\r\n"
        " [1]        : 0 degrees   (Min)\r\n"
        " [2]        : 45 degrees\r\n"
        " [3]        : 90 degrees  (Center)\r\n"
        " [4]        : 135 degrees\r\n"
        " [5]        : 180 degrees (Max)\r\n"
        " [0 - 180]  : Type any custom angle directly!\r\n"
        " [h]        : Show this Help menu\r\n"
        "==========================================\r\n"
        "Current Servo Angle: 90 degrees\r\n"
        "Enter command: "
    );
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

    if (!uart_is_driver_installed(UART_PORT)) {
        uart_driver_install(UART_PORT, BUF_SIZE, 0, 0, NULL, 0);
        uart_param_config(UART_PORT, &uart_config);
    }

    // Set non-blocking stdin
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags != -1) {
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    current_mode = UART_MODE_SERVO;
    current_servo_angle = 90;
    Servo_SetAngle(90);

    print_main_menu();
}

static char line_buffer[32];
static uint8_t line_idx = 0;

void UART_CONTROL_update(void) {
    uint8_t ch = 0;
    int len = 0;

    if (uart_is_driver_installed(UART_PORT)) {
        len = uart_read_bytes(UART_PORT, &ch, 1, 0);
    }

    if (len <= 0) {
        int r = read(STDIN_FILENO, &ch, 1);
        if (r > 0) {
            len = 1;
        }
    }

    if (len > 0) {
        // Handle newline / command submission
        if (ch == '\r' || ch == '\n') {
            if (line_idx > 0) {
                line_buffer[line_idx] = '\0';
                
                // 1. Check if multi-digit number entered (e.g. "120", "45", "180")
                char *endptr;
                long parsed_val = strtol(line_buffer, &endptr, 10);
                if (endptr != line_buffer && *endptr == '\0') {
                    if (parsed_val < 0) parsed_val = 0;
                    if (parsed_val > 180) parsed_val = 180;
                    current_servo_angle = (uint8_t)parsed_val;
                    Servo_SetAngle(current_servo_angle);
                    char buf[96];
                    snprintf(buf, sizeof(buf),
                             "\r\n-> [COMMAND] Set angle to: %d deg\r\nEnter command: ",
                             current_servo_angle);
                    UART_SendMessage(buf);
                } else if (strcmp(line_buffer, "h") == 0 || strcmp(line_buffer, "H") == 0) {
                    print_main_menu();
                } else if (strcmp(line_buffer, "a") == 0 || strcmp(line_buffer, "A") == 0) {
                    current_servo_angle = (current_servo_angle >= 5) ? current_servo_angle - 5 : 0;
                    Servo_SetAngle(current_servo_angle);
                    char buf[96];
                    snprintf(buf, sizeof(buf), "\r\n-> [COMMAND] Decreased to: %d deg\r\nEnter command: ", current_servo_angle);
                    UART_SendMessage(buf);
                } else if (strcmp(line_buffer, "d") == 0 || strcmp(line_buffer, "D") == 0) {
                    current_servo_angle = (current_servo_angle <= 175) ? current_servo_angle + 5 : 180;
                    Servo_SetAngle(current_servo_angle);
                    char buf[96];
                    snprintf(buf, sizeof(buf), "\r\n-> [COMMAND] Increased to: %d deg\r\nEnter command: ", current_servo_angle);
                    UART_SendMessage(buf);
                } else {
                    char buf[96];
                    snprintf(buf, sizeof(buf), "\r\n-> Unknown command '%s'. Type 'h' for help.\r\nEnter command: ", line_buffer);
                    UART_SendMessage(buf);
                }
                line_idx = 0;
            }
            return;
        }

        // Handle instant single keypress presets: 'a', 'd', '1'..'5', 'h'
        if (ch == 'a' || ch == 'A') {
            current_servo_angle = (current_servo_angle >= 5) ? current_servo_angle - 5 : 0;
            Servo_SetAngle(current_servo_angle);
            char buf[96];
            snprintf(buf, sizeof(buf), "\r\n-> [INSTANT] Angle: %d deg\r\nEnter command: ", current_servo_angle);
            UART_SendMessage(buf);
            line_idx = 0;
            return;
        } else if (ch == 'd' || ch == 'D') {
            current_servo_angle = (current_servo_angle <= 175) ? current_servo_angle + 5 : 180;
            Servo_SetAngle(current_servo_angle);
            char buf[96];
            snprintf(buf, sizeof(buf), "\r\n-> [INSTANT] Angle: %d deg\r\nEnter command: ", current_servo_angle);
            UART_SendMessage(buf);
            line_idx = 0;
            return;
        } else if (ch == '1' && line_idx == 0) {
            current_servo_angle = 0;
            Servo_SetAngle(0);
            char buf[96];
            snprintf(buf, sizeof(buf), "\r\n-> [PRESET 1] Angle: %d deg\r\nEnter command: ", current_servo_angle);
            UART_SendMessage(buf);
            return;
        } else if (ch == '2' && line_idx == 0) {
            current_servo_angle = 45;
            Servo_SetAngle(45);
            char buf[96];
            snprintf(buf, sizeof(buf), "\r\n-> [PRESET 2] Angle: %d deg\r\nEnter command: ", current_servo_angle);
            UART_SendMessage(buf);
            return;
        } else if (ch == '3' && line_idx == 0) {
            current_servo_angle = 90;
            Servo_SetAngle(90);
            char buf[96];
            snprintf(buf, sizeof(buf), "\r\n-> [PRESET 3] Angle: %d deg\r\nEnter command: ", current_servo_angle);
            UART_SendMessage(buf);
            return;
        } else if (ch == '4' && line_idx == 0) {
            current_servo_angle = 135;
            Servo_SetAngle(135);
            char buf[96];
            snprintf(buf, sizeof(buf), "\r\n-> [PRESET 4] Angle: %d deg\r\nEnter command: ", current_servo_angle);
            UART_SendMessage(buf);
            return;
        } else if (ch == '5' && line_idx == 0) {
            current_servo_angle = 180;
            Servo_SetAngle(180);
            char buf[96];
            snprintf(buf, sizeof(buf), "\r\n-> [PRESET 5] Angle: %d deg\r\nEnter command: ", current_servo_angle);
            UART_SendMessage(buf);
            return;
        } else if ((ch == 'h' || ch == 'H') && line_idx == 0) {
            print_main_menu();
            return;
        }

        // Buffer numeric characters for custom angle (e.g. typing "120")
        if (line_idx < sizeof(line_buffer) - 1) {
            line_buffer[line_idx++] = (char)ch;
        }
    }
}

UART_ControlMode UART_CONTROL_GetMode(void) {
    return current_mode;
}
