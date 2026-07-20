#include <main.h>
#include <servo.h>

extern TIM_HandleTypeDef htim3; //using timer 3

void servo_init (void) {
    //start pwm for timer 3
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
}

void Servo_SetAngle(uint8_t angle) {

    //no angle can be greater than 180 deg
    if (angle > 180) {
        angle = 180;
    }

    // Map 0 - 180 degrees to 500 - 2500 microseconds (compare value) to get full 180 degree travel
    // Formula: pulse = MinPulse + (angle * (MaxPulse - MinPulse) / 180)
    uint32_t compare_value = 500 + ((uint32_t)angle * 2000) / 180;

    //set the pulse width
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, compare_value);
}

void servo_stop(void) {
    //turn off pwm
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
}