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

    uint8_t rx_byte = 0;
    robot_status_t status_packet;

    while (1) {
        // Read incoming telemetry from STM32 on UART1 (Pin 2)
        int len = uart_read_bytes(UART_NUM_1, &rx_byte, 1, pdMS_TO_TICKS(10));
        if (len > 0 && rx_byte == 0xBB) {
            // Read full robot_status_t payload following the 0xBB header
            int status_len = uart_read_bytes(UART_NUM_1, (uint8_t*)&status_packet, sizeof(robot_status_t), pdMS_TO_TICKS(20));
            if (status_len == sizeof(robot_status_t)) {
                // Forward telemetry back to the transmitter via ESP-NOW
                if (g_transmitter_paired) {
                    esp_now_send(g_transmitter_mac, (uint8_t*)&status_packet, sizeof(robot_status_t));
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
