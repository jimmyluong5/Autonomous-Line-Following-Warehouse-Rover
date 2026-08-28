#include "speaker.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "stdbool.h"
#include "esp_log.h"

#define joystick_button GPIO_NUM_6
#define speaker_pin     GPIO_NUM_21

#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_DUTY_RES   LEDC_TIMER_10_BIT // 10-bit timer (0 to 1023 max)
#define LEDC_FREQUENCY  2048              // 2048 Hz tone

static const char *TAG = "SPEAKER";

bool is_beeping = false;

void init_speaker(void) {

    //configure the active-low button
    gpio_reset_pin(joystick_button);
    gpio_set_direction(joystick_button, GPIO_MODE_INPUT);
    gpio_set_pull_mode(joystick_button, GPIO_PULLUP_ONLY);

    //configure the ledc timer for the speaker
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);

    //configure the ledc channel for gpio 21 which is the speaker.
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = speaker_pin,
        .duty = 0,
        .hpoint = 0,
    };
    ledc_channel_config(&ledc_channel);

    //keep the pnp transistor off at startup
    ledc_stop(LEDC_MODE, LEDC_CHANNEL, 1);

}

void speaker_update(void) {
    //check if the button is pressed
    if (gpio_get_level(joystick_button) == 0 ) {
        //set this true is_beeping = true;
        is_beeping = true;
        //turn on the 2048 HZ tone generator for GPIO 20
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 512); //set the duty cycle to 50%
        //because our 100% is 2^10-1. or 1023. because its a 10 bit timer.
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        ESP_LOGI(TAG, "Speaker activated");

    }

    else {
        //turn off the speaker and set the flag false.
            //stop the tone and the set output pin high to keep pnp transistor off.
        ledc_stop(LEDC_MODE, LEDC_CHANNEL, 1);
        is_beeping = false; //set the flag off.
    }
}


