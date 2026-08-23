#include "uart_control.h"
#include "servo.h"
#include "driver/usb_serial_jtag.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UART_PORT UART_NUM_0
#define BUF_SIZE 512

static uint8_t current_servo_angle = SERVO_CENTER_ANGLE;
static UART_ControlMode current_mode = UART_MODE_SERVO;
static uint32_t last_manual_command_time = 0;
static bool manual_override_active = false;
static bool is_stopped = false;

static uint32_t get_time_ms(void) {
    return pdTICKS_TO_MS(xTaskGetTickCount());
}

static void UART_SendMessage(const char *message) {
    printf("%s", message);
    fflush(stdout);
}

void print_main_menu(void) {
    uint8_t p1 = SERVO_MIN_ANGLE;
    uint8_t p2 = SERVO_MIN_ANGLE + (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE) / 4;
    uint8_t p3 = SERVO_CENTER_ANGLE;
    uint8_t p4 = SERVO_MAX_ANGLE - (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE) / 4;
    uint8_t p5 = SERVO_MAX_ANGLE;

    char menu_buf[512];
    snprintf(menu_buf, sizeof(menu_buf),
        "\r\n==========================================\r\n"
        "   SUSPENSION ARM SERVO (%d° - %d°)\r\n"
        "==========================================\r\n"
        "Commands:\r\n"
        " [1]        : %d deg  (Min Safety Limit)\r\n"
        " [2]        : %d deg\r\n"
        " [3]        : %d deg  (Center)\r\n"
        " [4]        : %d deg\r\n"
        " [5]        : %d deg  (Max Safety Limit)\r\n"
        " [a] / [d]  : Step -5 deg / +5 deg (%d° - %d°)\r\n"
        " [t]        : TURN OFF / STOP Servo Motor\r\n"
        " [h]        : Show this Help menu\r\n"
        "==========================================\r\n"
        "Current Servo Angle: %d degrees\r\n",
        SERVO_MIN_ANGLE, SERVO_MAX_ANGLE,
        p1, p2, p3, p4, p5,
        SERVO_MIN_ANGLE, SERVO_MAX_ANGLE,
        Servo_GetCurrentAngle()
    );
    UART_SendMessage(menu_buf);
}

void UART_CONTROL_init(void) {
    usb_serial_jtag_driver_config_t usb_serial_config = {
        .tx_buffer_size = 1024,
        .rx_buffer_size = 1024,
    };
    usb_serial_jtag_driver_install(&usb_serial_config);

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

    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    current_mode = UART_MODE_SERVO;
    current_servo_angle = SERVO_CENTER_ANGLE;
    is_stopped = false;
    Servo_SetAngleImmediate(SERVO_CENTER_ANGLE);

    print_main_menu();
}

void UART_CONTROL_update(void) {
    uint8_t ch = 0;
    int len = 0;

    len = usb_serial_jtag_read_bytes(&ch, 1, 0);
    if (len <= 0 && uart_is_driver_installed(UART_PORT)) {
        len = uart_read_bytes(UART_PORT, &ch, 1, 0);
    }

    if (len > 0) {
        if (ch == '\r' || ch == '\n' || ch == ' ') {
            return;
        }

        manual_override_active = true;
        last_manual_command_time = get_time_ms();

        // [t] - Turn OFF servo completely until next input
        if (ch == 't' || ch == 'T') {
            is_stopped = true;
            servo_stop();
            char buf[96];
            snprintf(buf, sizeof(buf), "\r\n>>> [STOPPED] Servo motor TURNED OFF at %d deg! (Paused until next input)\r\n", Servo_GetCurrentAngle());
            UART_SendMessage(buf);
            return;
        }

        // Any other movement command cancels STOP and re-enables PWM
        is_stopped = false;

        uint8_t p1 = SERVO_MIN_ANGLE;
        uint8_t p2 = SERVO_MIN_ANGLE + (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE) / 4;
        uint8_t p3 = SERVO_CENTER_ANGLE;
        uint8_t p4 = SERVO_MAX_ANGLE - (SERVO_MAX_ANGLE - SERVO_MIN_ANGLE) / 4;
        uint8_t p5 = SERVO_MAX_ANGLE;

        if (ch == '1') {
            current_servo_angle = p1;
            Servo_SetAngleImmediate(p1);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Preset 1 -> Min Angle: %d deg\r\n", p1);
            UART_SendMessage(buf);
        } else if (ch == '2') {
            current_servo_angle = p2;
            Servo_SetAngleImmediate(p2);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Preset 2 -> Angle: %d deg\r\n", p2);
            UART_SendMessage(buf);
        } else if (ch == '3' || ch == 's' || ch == 'S') {
            current_servo_angle = p3;
            Servo_SetAngleImmediate(p3);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Preset 3 -> Center Angle: %d deg\r\n", p3);
            UART_SendMessage(buf);
        } else if (ch == '4') {
            current_servo_angle = p4;
            Servo_SetAngleImmediate(p4);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Preset 4 -> Angle: %d deg\r\n", p4);
            UART_SendMessage(buf);
        } else if (ch == '5') {
            current_servo_angle = p5;
            Servo_SetAngleImmediate(p5);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Preset 5 -> Max Angle: %d deg\r\n", p5);
            UART_SendMessage(buf);
        } else if (ch == 'a' || ch == 'A') {
            current_servo_angle = (current_servo_angle >= SERVO_MIN_ANGLE + 5) ? current_servo_angle - 5 : SERVO_MIN_ANGLE;
            Servo_SetAngleImmediate(current_servo_angle);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Step -5 deg -> Angle: %d deg\r\n", current_servo_angle);
            UART_SendMessage(buf);
        } else if (ch == 'd' || ch == 'D') {
            current_servo_angle = (current_servo_angle <= SERVO_MAX_ANGLE - 5) ? current_servo_angle + 5 : SERVO_MAX_ANGLE;
            Servo_SetAngleImmediate(current_servo_angle);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Step +5 deg -> Angle: %d deg\r\n", current_servo_angle);
            UART_SendMessage(buf);
        } else if (ch == 'h' || ch == 'H') {
            print_main_menu();
        }
    }

    // While stopped, hold indefinite pause until user gives new command
    if (is_stopped) {
        last_manual_command_time = get_time_ms();
    }

    // After 10 seconds of no manual commands, resume safe auto sweep
    if (!is_stopped && manual_override_active && (get_time_ms() - last_manual_command_time > 10000)) {
        manual_override_active = false;
        char buf[80];
        snprintf(buf, sizeof(buf), "\r\n[IDLE] Resuming Safe Auto-Sweep (%d°-%d°)...\r\n", SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
        UART_SendMessage(buf);
    }
}

bool UART_CONTROL_IsManualActive(void) {
    return manual_override_active || is_stopped;
}

UART_ControlMode UART_CONTROL_GetMode(void) {
    return current_mode;
}
