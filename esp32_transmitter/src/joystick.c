#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "joystick.h"
#include "adc.h"
//initialize the joysticks by calling the adc functions

uint16_t read_joystick_horizontal(void) {
    uint16_t value_x = read_joystick_x();
}

uint16_t read_joystick_vertical(void) {
    uint16_t value_y = read_joystick_y();
}
