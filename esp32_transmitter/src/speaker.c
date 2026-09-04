#include "speaker.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "stdbool.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define joystick_button GPIO_NUM_6
#define speaker_pin     GPIO_NUM_21

#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_DUTY_RES   LEDC_TIMER_10_BIT // 10-bit timer (0 to 1023 max)
#define LEDC_FREQUENCY  1024              // either 3072 or 1024 Hz
#define JOYSTICK_FREQ 2048 //or 2731 for loud
#define BUTTON_FREQ 1024

//static const char *TAG = "SPEAKER";

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

void speaker_update(uint8_t button_packet) {
    //check if the button is pressed
    int sample1 = 0;
    int sample2 = 0; //make the debouncing logic with this.
    
    if (gpio_get_level(joystick_button) == 0) {
        sample1 = 1;
    }
    
    //if the sample is actually valid then pause for 20ms to let the button
    //bounce back up
    if (sample1 == 1) {
        vTaskDelay(pdMS_TO_TICKS(50)); // debounce 20ms
    }

    if (gpio_get_level(joystick_button) == 0) {
        sample2 = 1;
    }

    bool joystick_pressed = (sample1 == 1 && sample2 == 1);
    bool normal_button_pressed = (button_packet != 0);

    if (joystick_pressed) {
        //turn on the is beeping flag
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, JOYSTICK_FREQ);
        speaker_on();
    }
    else if (normal_button_pressed) {
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, BUTTON_FREQ);
        speaker_on();
    }
    
    else {
        //turn off the speaker and set the flag false.
            //stop the tone and the set output pin high to keep pnp transistor off.
        speaker_off();
    }
}

void speaker_on(void){
    if (!is_beeping) {
        is_beeping = true;
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 512);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        //ESP_LOGI(TAG, "Speaker activated");
    }
}

void speaker_off(void){
    if (is_beeping) {
        ledc_stop(LEDC_MODE, LEDC_CHANNEL, 1);
        is_beeping = false; //set the flag off.
    }
}


