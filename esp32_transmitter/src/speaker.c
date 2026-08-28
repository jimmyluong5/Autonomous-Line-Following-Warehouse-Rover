#include <speaker.h>
#include "driver/gpio.h"
#include "driver/ledc.h"


#define joystick_button GPIO_NUM_7
#define speaker_pin     GPIO_NUM_20

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_DUTY_RES   LEC_TIMER_10_BIT
#define LEDC_FREQUENCY  2048 //2048 hz tone.

bool is_beeping = false; //turn it off at first.

void init_speaker(void) {

    //configure the active-low button
    gpio_reset_pin(joystick_button);
    gpio_set_direction(joystick_button, GPIO_MODE_INPUT);
    gpio_set_pull_mode(joystick_button, GPIO_PULLUP_ONLY);


    



}