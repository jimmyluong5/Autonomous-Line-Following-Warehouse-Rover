#include <main.h>
#include <servo.h>
#include <stdbool.h>

extern TIM_HandleTypeDef htim3; // using timer 3

static float current_angle = 90.0f;
static uint8_t target_angle = 90;
static uint32_t last_step_tick = 0;
static uint32_t last_move_tick = 0;
static bool pwm_active = false;
static uint16_t step_interval_ms = 3;       // 3ms interval for fast, responsive movement
static float step_size_deg = 2.0f;           // 2.0° per step -> 180° full turn in ~270ms
static uint16_t auto_detach_delay_ms = 500; // Turn off PWM 500ms after reaching target
static bool auto_detach_enabled = true;

static void update_compare_value(uint8_t angle) {
    if (angle < SERVO_ANGLE_MIN) {
        angle = SERVO_ANGLE_MIN;
    } else if (angle > SERVO_ANGLE_MAX) {
        angle = SERVO_ANGLE_MAX;
    }
    // Map 0 - 180 degrees to 500 - 2500 microseconds (compare value)
    uint32_t compare_value = 500 + ((uint32_t)angle * 2000) / 180;
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, compare_value);
}

void servo_init(void) {
    current_angle = 90.0f;
    target_angle = 90;
    last_step_tick = HAL_GetTick();
    last_move_tick = HAL_GetTick();
    update_compare_value(90);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    pwm_active = true;
}

void Servo_SetAngle(uint8_t angle) {
    if (angle < SERVO_ANGLE_MIN) {
        angle = SERVO_ANGLE_MIN;
    } else if (angle > SERVO_ANGLE_MAX) {
        angle = SERVO_ANGLE_MAX;
    }
    target_angle = angle;
    last_move_tick = HAL_GetTick();

    // Start PWM if currently stopped/detached
    if (!pwm_active) {
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
        pwm_active = true;
    }
}

void Servo_SetAngleImmediate(uint8_t angle) {
    if (angle < SERVO_ANGLE_MIN) {
        angle = SERVO_ANGLE_MIN;
    } else if (angle > SERVO_ANGLE_MAX) {
        angle = SERVO_ANGLE_MAX;
    }
    target_angle = angle;
    current_angle = (float)angle;
    last_move_tick = HAL_GetTick();

    if (!pwm_active) {
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
        pwm_active = true;
    }
    update_compare_value(angle);
}

void Servo_Update(void) {
    uint32_t now = HAL_GetTick();

    // Soft slew-rate ramping step
    if ((uint8_t)current_angle != target_angle) {
        if (now - last_step_tick >= step_interval_ms) {
            last_step_tick = now;
            last_move_tick = now;

            if (!pwm_active) {
                HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
                pwm_active = true;
            }

            if (current_angle < target_angle) {
                current_angle += step_size_deg;
                if (current_angle > target_angle) {
                    current_angle = (float)target_angle;
                }
            } else {
                current_angle -= step_size_deg;
                if (current_angle < target_angle) {
                    current_angle = (float)target_angle;
                }
            }

            update_compare_value((uint8_t)current_angle);
        }
    } else {
        // Target reached. Handle auto-detach / PWM disable to eliminate idle power draw & heating
        if (auto_detach_enabled && pwm_active && (now - last_move_tick >= auto_detach_delay_ms)) {
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
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
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
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