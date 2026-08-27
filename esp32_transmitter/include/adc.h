#ifndef ADC_H
#define ADC_H

#include "esp_adc/adc_oneshot.h"

// ADC channel mapping for ESP32-S3 (Freenove ESP32-S3 WROOM)
// GPIO 5 -> ADC1 Channel 4 (Joystick X)
// GPIO 4 -> ADC1 Channel 3 (Joystick Y)
#define joystick_x ADC_CHANNEL_4
#define joystick_y ADC_CHANNEL_3

/**
 * @brief Initialize ADC Unit 1 and configure channels for Joystick X and Y
 */
void init_adc(void);

/**
 * @brief Read raw ADC value for Joystick X axis
 * @return int Raw ADC reading (0 - 4095)
 */
int read_joystick_x(void);

/**
 * @brief Read raw ADC value for Joystick Y axis
 * @return int Raw ADC reading (0 - 4095)
 */
int read_joystick_y(void);

#endif // ADC_H
