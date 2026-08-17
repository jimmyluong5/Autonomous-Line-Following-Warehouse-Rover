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
    UART_SendMessage(
        "\r\n==========================================\r\n"
        "   SUSPENSION ARM SERVO CONTROLLER (40°-140°)\r\n"
        "==========================================\r\n"
        "Commands (Tap key in Monitor):\r\n"
        " [1]        : 40 degrees  (Min Safety Limit)\r\n"
        " [2]        : 65 degrees\r\n"
        " [3]        : 90 degrees  (Center)\r\n"
        " [4]        : 115 degrees\r\n"
        " [5]        : 140 degrees (Max Safety Limit)\r\n"
        " [a] / [d]  : Step -5 deg / +5 deg (40° - 140°)\r\n"
        " [t]        : STOP / HOLD Servo Motion\r\n"
        " [h]        : Show this Help menu\r\n"
        "==========================================\r\n"
        "Current Servo Angle: 90 degrees\r\n"
    );
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

        // Any command cancels the STOP state and engages manual override
        manual_override_active = true;
        last_manual_command_time = get_time_ms();

        if (ch == 't' || ch == 'T') {
            is_stopped = true;
            servo_stop();
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [STOP] Servo Motion STOPPED at %d deg!\r\n", Servo_GetCurrentAngle());
            UART_SendMessage(buf);
            return;
        }

        is_stopped = false;

        if (ch == '1') {
            current_servo_angle = 40;
            Servo_SetAngleImmediate(40);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Preset 1 -> Min Angle: 40 deg\r\n");
            UART_SendMessage(buf);
        } else if (ch == '2') {
            current_servo_angle = 65;
            Servo_SetAngleImmediate(65);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Preset 2 -> Angle: 65 deg\r\n");
            UART_SendMessage(buf);
        } else if (ch == '3' || ch == 's' || ch == 'S') {
            current_servo_angle = 90;
            Servo_SetAngleImmediate(90);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Preset 3 -> Center Angle: 90 deg\r\n");
            UART_SendMessage(buf);
        } else if (ch == '4') {
            current_servo_angle = 115;
            Servo_SetAngleImmediate(115);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Preset 4 -> Angle: 115 deg\r\n");
            UART_SendMessage(buf);
        } else if (ch == '5') {
            current_servo_angle = 140;
            Servo_SetAngleImmediate(140);
            char buf[80];
            snprintf(buf, sizeof(buf), "\r\n>>> [MANUAL] Preset 5 -> Max Angle: 140 deg\r\n");
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

    // If stopped, keep manual override active indefinitely until a new key is pressed
    if (is_stopped) {
        last_manual_command_time = get_time_ms();
    }

    // After 10 seconds of no manual commands, resume safe auto sweep (40°-140°)
    if (!is_stopped && manual_override_active && (get_time_ms() - last_manual_command_time > 10000)) {
        manual_override_active = false;
        UART_SendMessage("\r\n[IDLE] Resuming Safe Auto-Sweep (40°-140°)...\r\n");
    }
}

bool UART_CONTROL_IsManualActive(void) {
    return manual_override_active || is_stopped;
}

UART_ControlMode UART_CONTROL_GetMode(void) {
    return current_mode;
}
