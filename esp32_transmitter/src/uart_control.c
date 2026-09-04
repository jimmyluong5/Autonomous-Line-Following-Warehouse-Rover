#include "uart_control.h"
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

void UART_SendMessage(const char *message) {
    printf("%s", message);
    fflush(stdout);
}

void print_main_menu(void) {
    char menu_buf[256];
    snprintf(menu_buf, sizeof(menu_buf),
        "\r\n==========================================\r\n"
        "   ESP32 SERIAL CONSOLE READY            \r\n"
        "==========================================\r\n"
        "Commands:\r\n"
        " [h] : Show this Help menu\r\n"
        "==========================================\r\n"
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
        if (ch == '\r' || ch == '\n') {
            return;
        }

        if (ch == 'h' || ch == 'H') {
            print_main_menu();
        } else {
            char buf[64];
            snprintf(buf, sizeof(buf), "Received command: '%c' (0x%02X)\r\n", ch, ch);
            UART_SendMessage(buf);
        }
    }
}
