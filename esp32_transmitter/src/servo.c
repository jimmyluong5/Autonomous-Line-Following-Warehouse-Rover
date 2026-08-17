#include "servo.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>
#include <stdint.h>

#define SERVO_PIN GPIO_NUM_14
#define SERVO_MODE LEDC_LOW_SPEED_MODE
#define SERVO_CHANNEL LEDC_CHANNEL_0
#define SERVO_TIMER LEDC_TIMER_0
#define SERVO_DUTY_RES LEDC_TIMER_13_BIT
#define SERVO_FREQ_HZ 50

static const char *TAG = "SERVO";

static float current_angle = (float)SERVO_CENTER_ANGLE;
static uint8_t target_angle = SERVO_CENTER_ANGLE;
static uint32_t last_step_tick = 0;
static uint32_t last_move_tick = 0;
static bool pwm_active = false;
static uint16_t step_interval_ms = 10;       // 10ms update step
static float step_size_deg = 5.0f;           // 5.0° per step
static uint16_t auto_detach_delay_ms = 500;
static bool auto_detach_enabled = false;    // Keep continuous PWM to hold position

static uint32_t get_tick_ms(void) {
    return pdTICKS_TO_MS(xTaskGetTickCount());
}

static uint8_t clamp_angle(uint8_t angle) {
    if (angle < SERVO_MIN_ANGLE) {
        return SERVO_MIN_ANGLE;
    }
    if (angle > SERVO_MAX_ANGLE) {
        return SERVO_MAX_ANGLE;
    }
    return angle;
}

static void update_compare_value(uint8_t angle) {
    angle = clamp_angle(angle);

    // Map 0 - 180 degrees to 500 - 2500 microseconds pulse width
    // 13-bit timer at 50 Hz -> 20ms period = 8191 counts
    uint32_t pulse_us = 500 + ((uint32_t)angle * 2000) / 180;
    uint32_t duty = (pulse_us * 8191) / 20000;

    ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, duty);
    ledc_update_duty(SERVO_MODE, SERVO_CHANNEL);
}

void servo_init(void) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = SERVO_MODE,
        .timer_num        = SERVO_TIMER,
        .duty_resolution  = SERVO_DUTY_RES,
        .freq_hz          = SERVO_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = SERVO_MODE,
        .channel        = SERVO_CHANNEL,
        .timer_sel      = SERVO_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO_PIN,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    current_angle = (float)SERVO_CENTER_ANGLE;
    target_angle = SERVO_CENTER_ANGLE;
    last_step_tick = get_tick_ms();
    last_move_tick = get_tick_ms();
    update_compare_value(SERVO_CENTER_ANGLE);
    pwm_active = true;
    ESP_LOGI(TAG, "Servo initialized on GPIO %d (Clamped %d° - %d°)", SERVO_PIN, SERVO_MIN_ANGLE, SERVO_MAX_ANGLE);
}

void Servo_SetAngle(uint8_t angle) {
    target_angle = clamp_angle(angle);
    last_move_tick = get_tick_ms();
    pwm_active = true;
}

void Servo_SetAngleImmediate(uint8_t angle) {
    angle = clamp_angle(angle);
    target_angle = angle;
    current_angle = (float)angle;
    last_move_tick = get_tick_ms();
    pwm_active = true;

    update_compare_value(angle);
}

void Servo_Update(void) {
    uint32_t now = get_tick_ms();

    if ((uint8_t)current_angle != target_angle) {
        if (now - last_step_tick >= step_interval_ms) {
            last_step_tick = now;
            last_move_tick = now;
            pwm_active = true;

            if (current_angle < (float)target_angle) {
                current_angle += step_size_deg;
                if (current_angle > (float)target_angle) {
                    current_angle = (float)target_angle;
                }
            } else {
                current_angle -= step_size_deg;
                if (current_angle < (float)target_angle) {
                    current_angle = (float)target_angle;
                }
            }

            // Ensure floating current_angle stays within bounds
            if (current_angle < (float)SERVO_MIN_ANGLE) current_angle = (float)SERVO_MIN_ANGLE;
            if (current_angle > (float)SERVO_MAX_ANGLE) current_angle = (float)SERVO_MAX_ANGLE;

            update_compare_value((uint8_t)current_angle);
        }
    } else {
        if (auto_detach_enabled && pwm_active && (now - last_move_tick >= auto_detach_delay_ms)) {
            ledc_stop(SERVO_MODE, SERVO_CHANNEL, 0);
            pwm_active = false;
        }
    }
}

void Servo_SetSpeed(uint16_t interval_ms, float step_deg) {
    if (interval_ms < 1) interval_ms = 1;
    if (step_deg < 0.1f) step_deg = 0.1f;
    step_interval_ms = interval_ms;
    step_size_deg = step_deg;
}

void Servo_SetAutoDetach(bool enable, uint16_t delay_ms) {
    auto_detach_enabled = enable;
    auto_detach_delay_ms = delay_ms;
}

void servo_stop(void) {
    ledc_stop(SERVO_MODE, SERVO_CHANNEL, 0);
    pwm_active = false;
}

uint8_t Servo_GetTargetAngle(void) {
    return target_angle;
}

uint8_t Servo_GetCurrentAngle(void) {
    return (uint8_t)current_angle;
}

bool Servo_IsMoving(void) {
    return ((uint8_t)current_angle != target_angle);
}
