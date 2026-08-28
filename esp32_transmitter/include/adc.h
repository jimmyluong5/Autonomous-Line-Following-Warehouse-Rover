#ifndef ADC_H
#define ADC_H

#include "esp_adc/adc_oneshot.h"

// ADC channel mapping for ESP32-S3 (Freenove ESP32-S3 WROOM)
// GPIO 5 -> ADC1 Channel 4 (Joystick X)
// GPIO 4 -> ADC1 Channel 3 (Joystick Y)
#define JOYSTICK_X_CHANNEL ADC_CHANNEL_4
#define JOYSTICK_Y_CHANNEL ADC_CHANNEL_3

//initialize the adc
void init_adc(void);


int read_joystick_x(void);

int read_joystick_y(void);

#endif // ADC_H
