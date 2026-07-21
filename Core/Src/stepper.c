#include <main.h>
#include <stepper.h>

#define stepper_gear_ratio 100.0f
#define base_step_angle 18.0f
#define output_step_angle (base_step_angle / stepper_gear_ratio) //0.18 degrees.
#define step_per_revolution (360 / output_step_angle) //2000 steps per rev.

extern TIM_HandleTypeDef htim1; //timer 1

//using timer 1
void stepper_init(void) {
    // Disable driver outputs initially (active high to disable)
    HAL_GPIO_WritePin(Stepper_ENABLE_GPIO_Port, Stepper_ENABLE_Pin, GPIO_PIN_SET);
    
    // Set the direction pin low for now (default)
    HAL_GPIO_WritePin(Stepper_DIR_GPIO_Port, Stepper_DIR_Pin, GPIO_PIN_RESET);
}

//function to set/toggle the direction of the stepper.
void stepper_set_dir(void) {
    HAL_GPIO_TogglePin(Stepper_DIR_GPIO_Port, Stepper_DIR_Pin);
}

//function for moving the stepper motor starting the PWM
void stepper_step(void) {
    stepper_move_steps(1, 1000);
}

//function to move the stepper motor in one direction for a certain number of steps.
void stepper_move_steps (int32_t steps, uint32_t speed) {
    
    //check if the steps and speed is 0
    if (speed == 0 || steps == 0) {
        return;
    }

    //set the direction based on step sign.
    if (steps > 0) {
        //if steps>0 then we are CW
        //set the dir pin low
        HAL_GPIO_WritePin(Stepper_DIR_GPIO_Port, Stepper_DIR_Pin, GPIO_PIN_RESET);
    }
    else {
        //set the dir pin high
        HAL_GPIO_WritePin(Stepper_DIR_GPIO_Port, Stepper_DIR_Pin, GPIO_PIN_SET);
        steps = -steps; //make step count positive for the loop/delay
    }
 
    // Configure TIM1's Auto-Reload Register (ARR) for dynamic speed
    // Input clock = 1 MHz (Prescaler = 169). ARR = (1,000,000 / speed) - 1.
    uint32_t arr = (1000000 / speed) - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim1, arr); 
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (arr + 1) / 2); //50% duty cycle.
    
    // Clear update flag initially and reset counter
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
    __HAL_TIM_SET_COUNTER(&htim1, 0);

    //enable A4988 outputs (active-low)
    HAL_GPIO_WritePin(Stepper_ENABLE_GPIO_Port, Stepper_ENABLE_Pin, GPIO_PIN_RESET);
    HAL_Delay(1); // Small wake-up delay for driver IC

    //start the pwm 
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

    for (int32_t i = 0; i < steps; i++) {
        while (__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_UPDATE) == RESET) {
            // Wait for step period to complete
        }
        __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
        (void)htim1.Instance->SR; // Force bus synchronization to prevent double-triggering
    }

    //stop the pwm and the counting
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);

    //disable motor driver again.
    HAL_GPIO_WritePin(Stepper_ENABLE_GPIO_Port, Stepper_ENABLE_Pin, GPIO_PIN_SET);
}

// Rotate the output shaft by a specific angle in degrees (positive = CW, negative = CCW)
void stepper_move_degrees(float degrees, uint32_t speed) {
    // FIT0503 step angle is 0.18 degrees per step.
    // Calculate required steps = Degrees / 0.18
    float calculated_steps = degrees / output_step_angle;
    
    // Round to the nearest whole step count
    int32_t steps = (int32_t)(calculated_steps >= 0.0f ? (calculated_steps + 0.5f) : (calculated_steps - 0.5f));
    
    stepper_move_steps(steps, speed);
}

void stepper_stop(void) {
    //just turn off pwm, and set enable high to disable stepper motor
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_GPIO_WritePin(Stepper_ENABLE_GPIO_Port, Stepper_ENABLE_Pin, GPIO_PIN_SET);
}