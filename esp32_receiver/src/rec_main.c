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

extern uint8_t transmitter_mac[6];
extern bool g_transmitter_paired;

void app_main() {
    init_led();
    init_esp_nvs();
    init_wifi();
    init_pins();
    init_uart();
    init_esp_now();

    ESP_LOGI(TAG, "=============================================");
    ESP_LOGI(TAG, "   ROVER RECEIVER READY (UART1 <-> STM32)     ");
    ESP_LOGI(TAG, "   TX: GPIO 42 -> STM32 PA10 (RX)             ");
    ESP_LOGI(TAG, "   RX: GPIO 2  <- STM32 PA9  (TX)             ");
    ESP_LOGI(TAG, "=============================================");

    while (1) {
        uint8_t header = 0;
        // Check if marker byte 0xAA arrived from STM32 PA9
        if (uart_read_bytes(UART_NUM_1, &header, 1, pdMS_TO_TICKS(10)) > 0) {
            if (header == 0xAA) {
                robot_status_t status = {0};
                int bytes_read = uart_read_bytes(UART_NUM_1, (uint8_t*)&status, sizeof(robot_status_t), pdMS_TO_TICKS(20));
                
                if (bytes_read == sizeof(robot_status_t)) {
                    // Send live telemetry to the Transmitter over ESP-NOW!
                    esp_err_t err = esp_now_send(transmitter_mac, (const uint8_t*)&status, sizeof(robot_status_t));
                    if (err == ESP_OK) {
                        ESP_LOGI(TAG, "Forwarded Telemetry -> Transmitter (CPU: %u%%)", status.cpuLoad);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
