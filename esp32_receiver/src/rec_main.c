#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "setup.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_now.h"
#include "receive_data.h"
#include "driver/gpio.h"
#include "driver/uart.h"

static const char *TAG = "ROVER_RECEIVER";

extern uint8_t g_transmitter_mac[6];
extern bool g_transmitter_paired;

void app_main() {
    init_led();
    init_esp_nvs();
    init_wifi();
    init_pins();
    init_uart();
    init_esp_now(); // registers OnDataRecv to capture incoming transmitter packets

    ESP_LOGI(TAG, "=============================================");
    ESP_LOGI(TAG, "   ROVER RECEIVER READY (UART1 <-> STM32)     ");
    ESP_LOGI(TAG, "   TX: GPIO 42 -> STM32 PA3 (RX)              ");
    ESP_LOGI(TAG, "   RX: GPIO 2  <- STM32 PA2 (TX)              ");
    ESP_LOGI(TAG, "=============================================");

    uint8_t rx_buf[160];

    while (1) {
        // Read incoming responses / debug prints from STM32 on UART1 (Pin 2)
        int len = uart_read_bytes(UART_NUM_1, rx_buf, sizeof(rx_buf) - 1, pdMS_TO_TICKS(10));
        if (len > 0) {
            rx_buf[len] = '\0';
            // Print STM32's direct serial response to PuTTY
            printf("%s", (char*)rx_buf);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
