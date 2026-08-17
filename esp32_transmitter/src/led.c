#include "led.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const gpio_num_t LED_PINS[] = {GPIO_NUM_2, GPIO_NUM_21,
                                      GPIO_NUM_38, GPIO_NUM_47};
#define NUM_LEDS (sizeof(LED_PINS) / sizeof(LED_PINS[0]))

static const char *TAG = "LED";
static uint8_t s_led_state = 0;

void init_led(void) {
  ESP_LOGI(TAG, "Configuring onboard LED pins...");
  for (int i = 0; i < NUM_LEDS; i++) {
    gpio_reset_pin(LED_PINS[i]);
    gpio_set_direction(LED_PINS[i], GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PINS[i], 0);
  }
}

void blink_led(void) {
  s_led_state = !s_led_state;
  for (int i = 0; i < NUM_LEDS; i++) {
    gpio_set_level(LED_PINS[i], s_led_state);
  }
}

void set_led(bool on) {
  s_led_state = on ? 1 : 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    gpio_set_level(LED_PINS[i], s_led_state);
  }
}
