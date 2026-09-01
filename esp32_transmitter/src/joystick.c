#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "joystick.h"
#include "adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "JOYSTICK";

//initialize the joysticks by calling the adc functions

void init_joystick(void){
    init_adc();
}


uint16_t read_joystick_horizontal(void) {
    uint16_t value_x = read_joystick_x();
    return 4095-value_x;
}

uint16_t read_joystick_vertical(void) {
    uint16_t value_y = read_joystick_y();
    return value_y;
}

void print_joystick_values(void) {

    static uint32_t last_print_time = 0; 
    uint32_t now = pdTICKS_TO_MS(xTaskGetTickCount()); //current time in milliseconds

    if (now - last_print_time >= 400) { //has 200ms has passed?
        last_print_time = now; //remember when we last printed.

        uint16_t x = read_joystick_horizontal();
        uint16_t y = read_joystick_vertical();
        //ESP_LOGI(TAG, "Joystick X: %u | Y: %u", x, y);
    }


}