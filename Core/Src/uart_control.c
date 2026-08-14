#include "robot.h"
#include <MCP3208.h>
#include <encoder.h>
#include <line_following.h>
#include <main.h>
#include <servo.h>
#include <stdbool.h>
#include <stdio.h>
#include <stepper.h>
#include <string.h>
#include <uart_control.h>

static uint8_t current_servo_angle = 90;
static int16_t current_stepper_angle =
    0;                       // track current stepper angle starting at 0 deg.
#define BLACK_THRESHOLD 2359 // 1.90V on 3.3V ADC

extern UART_HandleTypeDef hcom_uart[];
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim4;

static uint32_t last_command_time = 0;

// flag for sensor test
static bool sensor_test_active = false;

// flag for the first print in the sensor test mode.
static bool first_print = true;

// flag for the control mode
static UART_ControlMode current_mode = UART_MODE_MENU;

// declare max and min variables later to calculate the max and min of each
// channel.
static uint16_t min[8];
static uint16_t max[8];
static uint16_t filtered_adc[8];

// function to set a message in UART.
static void UART_SendMessage(const char *message) {
  HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)message, strlen(message),
                    HAL_MAX_DELAY);
}

static uint32_t led_blink_start_time = 0;
static bool led_is_blinking = false;

// function to initiate the UART.
void UART_CONTROL_init(void) {
  current_mode = UART_MODE_MENU;
  sensor_test_active = false; // have the sensor test off at the

  char menu_buf[768];
  int percent = (robot_speed * 100) / 999;
  snprintf(menu_buf, sizeof(menu_buf),
           "\x1b[2J\x1b[H"
           "==========================================\r\n"
           "       Warehouse Rover Control Menu\r\n"
           "==========================================\r\n"
           "Select Option/Mode:\r\n"
           " [m] - Motor Control Mode\r\n"
           " [v] - Voltage / IR Sensor Test Mode\r\n"
           " [b] - Both Mode (Control + Sensor Prints)\r\n"
           " [n] - Normalized Color Mode\r\n"
           " [a] - Autonomous Line Following Mode\r\n"
           " [s] - Servo Control Mode\r\n"
           " [t] - Stepper Mode \r\n"
           " [p] - Speaker/Buzzer Test Mode\r\n"
           "------------------------------------------\r\n"
           "Select Speed (Current: %d%%):\r\n"
           " [1] - Set Speed to 25%% PWM\r\n"
           " [2] - Set Speed to 50%% PWM\r\n"
           " [3] - Set Speed to 75%% PWM\r\n"
           " [4] - Set Speed to 100%% PWM\r\n"
           "==========================================\r\n",
           percent);
  UART_SendMessage(menu_buf);
}

// function update the UART based on inputs through UART.
void UART_CONTROL_update(void) {
  // 8 bit byte to receive the inputs.
  uint8_t received_byte;

  // Handle non-blocking keypress LED turn-off after 50ms
  if (led_is_blinking && (HAL_GetTick() - led_blink_start_time >= 50)) {
    // Only turn off if the robot is currently idle or in fault state (other
    // states keep it ON)
    if (Robot_GetState() == robot_idle || Robot_GetState() == robot_fault) {
      HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
    }
    led_is_blinking = false;
  }

  // Clear any overrun or error flags that lock up UART reception
  if (__HAL_UART_GET_FLAG(&hcom_uart[COM1], UART_FLAG_ORE)) {
    __HAL_UART_CLEAR_OREFLAG(&hcom_uart[COM1]);
  }
  if (__HAL_UART_GET_FLAG(&hcom_uart[COM1], UART_FLAG_NE) ||
      __HAL_UART_GET_FLAG(&hcom_uart[COM1], UART_FLAG_FE) ||
      __HAL_UART_GET_FLAG(&hcom_uart[COM1], UART_FLAG_PE)) {
    __HAL_UART_CLEAR_FLAG(&hcom_uart[COM1],
                          UART_CLEAR_NEF | UART_CLEAR_FEF | UART_CLEAR_PEF);
  }

  // Check if one keyboard character was received (non-blocking)
  if (HAL_UART_Receive(&hcom_uart[COM1], &received_byte, 1, 0) == HAL_OK) {
    // Turn on the LED to indicate keypress
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
    led_blink_start_time = HAL_GetTick();
    led_is_blinking = true;

    // Update the last command timestamp
    last_command_time = HAL_GetTick();

    if (current_mode == UART_MODE_MENU) {
      if (received_byte == 'm') {
        current_mode = UART_MODE_MOTOR;
        UART_SendMessage(
            "\x1b[2J\x1b[H"
            "--- Motor Control Mode Active ---\r\n"
            "Commands:\r\n"
            " [w] - Forward\r\n"
            " [s] - Reverse\r\n"
            " [a] - Spin Turn Left\r\n"
            " [d] - Spin Turn Right\r\n"
            " [x] - Stop / Idle\r\n"
            " [f] - Force Fault\r\n"
            " [1, 2, 3, 4] - Set Speed to 25%, 50%, 75%, 100% PWM\r\n"
            " [h] - Return to Main Menu\r\n"
            "---------------------------------\r\n");
      }
      // just add new modes above here its easier up here
      else if (received_byte == 'p') {
        current_mode = UART_MODE_SPEAKER;
        first_print = true;
        sensor_test_active = false;
        UART_SendMessage("\x1b[2J\x1b[H"
                         "--- Speaker/Buzzer Test Mode Active ---\r\n"
                         "Press keys to test:\r\n"
                         " [1] - Play continuous 1 kHz tone\r\n"
                         " [2] - Play 100ms beep\r\n"
                         " [0] - Stop sound\r\n"
                         " [h] - Return to Main Menu\r\n"
                         "---------------------------------------\r\n");
      }

      else if (received_byte == 't') {
        // set the current mode to stepper mode
        current_mode = UART_MODE_STEPPER;
        first_print = true;
        // reset stepper angle to 0
        current_stepper_angle = 0;
        stepper_init();

        // put the custom menu for stepper motor here
        UART_SendMessage("\x1b[2J\x1b[H"
                         "--- Stepper Control Mode Active ---\r\n"
                         "Controls:\r\n"
                         " [a] - Decrease angle by 5 deg (CCW)\r\n"
                         " [d] - Increase angle by 5 deg (CW)\r\n"
                         " [1] - Set to 0 deg | [2] - Set to 180 deg | [3] - "
                         "Set to 360 deg\r\n"
                         " [h] - Return to Main Menu\r\n"
                         "-----------------------------------\r\n"
                         "Current Angle:    0 degrees");

      }

      // normalized mode
      else if (received_byte == 'n') {
        // set the current mode to normalize_mode
        current_mode = UART_MODE_NORMALIZE;
        // use a flag to determine whether to print the header or not.
        first_print = true;

        // turn on the sensor
        sensor_test_active = true;

        // print the message.
        UART_SendMessage(
            "\x1b[2J\x1b[H"
            "--- Normalized Color Mode Active (Press 'h' to return "
            "to Main Menu) ---\r\n");

      }

      // autonomous line following mode
      else if (received_byte == 'a') {
        current_mode = UART_MODE_AUTO;
        first_print = true;
        sensor_test_active = false;
        Robot_SetState(robot_auto);
        UART_SendMessage(
            "\x1b[2J\x1b[H"
            "--- Autonomous Line Following Active ---\r\n"
            "Commands:\r\n"
            " [1, 2, 3, 4] - Set Speed (25%, 50%, 75%, 100% PWM)\r\n"
            " [h]          - Stop & Return to Main Menu\r\n"
            "----------------------------------------\r\n");
      }

      // servo control mode
      else if (received_byte == 's') {
        current_mode = UART_MODE_SERVO;
        first_print = true;
        sensor_test_active = false;
        current_servo_angle = 90;
        Servo_SetAngle(current_servo_angle);
        UART_SendMessage("\x1b[2J\x1b[H"
                         "--- Servo Control Mode Active ---\r\n"
                         "Controls:\r\n"
                         " [a] - Decrease angle by 5 deg\r\n"
                         " [d] - Increase angle by 5 deg\r\n"
                         " [1] - Set to 0 deg | [2] - Set to 45 deg | [3] - "
                         "Set to 90 deg \r\n"
                         " [4] - Set to 135 deg | [5] - Set to 180 deg\r\n"
                         " [h] - Return to Main Menu\r\n"
                         "---------------------------------\r\n"
                         "Current Angle:  90 degrees");
      }

      // voltage mode
      else if (received_byte == 'v') {
        // set the current mode as the voltage mode
        current_mode = UART_MODE_VOLTAGE;

        // turn on the sensor test.
        sensor_test_active = true;

        // use a flag to determine whether to print the header or not.
        first_print = true;

        // print the stuff.
        UART_SendMessage("\x1b[2J\x1b[H"
                         "--- Sensor Test Mode Active (Press 'h' to return "
                         "to Main Menu) ---\r\n");
      }

      else if (received_byte == 'b') {
        // set the mode to both.
        current_mode = UART_MODE_BOTH;

        // turn on the sensor test.
        sensor_test_active = true;

        // use a flag to print the header or not.
        first_print = true;

        // print the menu in this mode.
        UART_SendMessage(
            "\x1b[2J\x1b[H"
            "--- Both Mode Active ---\r\n"
            "Motor Commands:\r\n"
            " [w] - Forward | [s] - Reverse | [a] - Left | [d] - Right | [x] - "
            "Stop\r\n"
            " [1, 2, 3, 4] - Set Speed to 25%, 50%, 75%, 100% PWM\r\n"
            " [h] - Return to Main Menu\r\n"
            "---------------------------------\r\n");

      }

      else if (received_byte == '1') {
        robot_speed = 250;
        UART_SendMessage("\r\nSpeed set to 25% PWM (250/999)\r\n");
        UART_CONTROL_init();
      }

      else if (received_byte == '2') {
        robot_speed = 500;
        UART_SendMessage("\r\nSpeed set to 50% PWM (500/999)\r\n");
        UART_CONTROL_init();
      }

      else if (received_byte == '3') {
        robot_speed = 750;
        UART_SendMessage("\r\nSpeed set to 75% PWM (750/999)\r\n");
        UART_CONTROL_init();
      }

      else if (received_byte == '4') {
        robot_speed = 999;
        UART_SendMessage("\r\nSpeed set to 100% PWM (999/999)\r\n");
        UART_CONTROL_init();
      }

      else {
        UART_CONTROL_init(); // Reprint menu on invalid key
      }

    }

    else if (current_mode == UART_MODE_MOTOR ||
             current_mode == UART_MODE_BOTH) {

      if (received_byte == 'h') {
        Robot_SetState(robot_idle);
        sensor_test_active = false;
        UART_SendMessage("\r\nExiting Mode. Stopping Robot.\r\n");
        UART_CONTROL_init();
      }

      else if (received_byte == 'w') {
        Robot_SetState(robot_forward);
        if (current_mode == UART_MODE_MOTOR)
          UART_SendMessage("ROBOT FORWARD\r\n");
      }

      else if (received_byte == 's') {
        Robot_SetState(robot_reverse);
        if (current_mode == UART_MODE_MOTOR)
          UART_SendMessage("ROBOT REVERSE\r\n");
      }

      else if (received_byte == 'x') {
        Robot_SetState(robot_idle);
        if (current_mode == UART_MODE_MOTOR)
          UART_SendMessage("ROBOT STOPPED\r\n");
      }

      else if (received_byte == 'a') {
        Robot_SetState(robot_left);
        if (current_mode == UART_MODE_MOTOR)
          UART_SendMessage("ROBOT LEFT\r\n");
      }

      else if (received_byte == 'd') {
        Robot_SetState(robot_right);
        if (current_mode == UART_MODE_MOTOR)
          UART_SendMessage("ROBOT RIGHT\r\n");
      }

      else if (received_byte == 'f') {
        Robot_SetState(robot_fault);
        if (current_mode == UART_MODE_MOTOR)
          UART_SendMessage("ROBOT FAULT\r\n");
      }

      else if (received_byte == '1') {
        robot_speed = 250;
        Robot_SetState(Robot_GetState());
        if (current_mode == UART_MODE_MOTOR)
          UART_SendMessage("Speed set to 25% PWM (250/999)\r\n");
      }

      else if (received_byte == '2') {
        robot_speed = 500;
        Robot_SetState(Robot_GetState());
        if (current_mode == UART_MODE_MOTOR)
          UART_SendMessage("Speed set to 50% PWM (500/999)\r\n");
      }

      else if (received_byte == '3') {
        robot_speed = 750;
        Robot_SetState(Robot_GetState());
        if (current_mode == UART_MODE_MOTOR)
          UART_SendMessage("Speed set to 75% PWM (750/999)\r\n");
      }

      else if (received_byte == '4') {
        robot_speed = 999;
        Robot_SetState(Robot_GetState());
        if (current_mode == UART_MODE_MOTOR)
          UART_SendMessage("Speed set to 100% PWM (999/999)\r\n");
      }

    }

    else if (current_mode == UART_MODE_VOLTAGE) {

      if (received_byte == 'h' || received_byte == 'v' ||
          received_byte == 'x') {
        sensor_test_active = false;
        UART_SendMessage("\r\n--- Exited Sensor Test Mode ---\r\n");
        UART_CONTROL_init();
      }
    }

    else if (current_mode == UART_MODE_NORMALIZE) {

      // add key binds to exit this mode
      if (received_byte == 'h' || received_byte == 'v' ||
          received_byte == 'x') {
        // turn off the sensor
        sensor_test_active = false;

        // send a message through UART
        UART_SendMessage("\r\n--- Exited Sensor Test Mode ---\r\n");

        // reset and reinitialize UART
        UART_CONTROL_init();
      }
    }

    else if (current_mode == UART_MODE_AUTO) {
      if (received_byte == 'h') {
        Robot_SetState(robot_idle);
        UART_SendMessage(
            "\r\n--- Exited Autonomous Mode. Stopping Robot. ---\r\n");
        UART_CONTROL_init();
      } else if (received_byte == '1') {
        robot_speed = 250;
        UART_SendMessage("Speed set to 25% PWM (250/999)\r\n");
      } else if (received_byte == '2') {
        robot_speed = 500;
        UART_SendMessage("Speed set to 50% PWM (500/999)\r\n");
      } else if (received_byte == '3') {
        robot_speed = 750;
        UART_SendMessage("Speed set to 75% PWM (750/999)\r\n");
      } else if (received_byte == '4') {
        robot_speed = 999;
        UART_SendMessage("Speed set to 100% PWM (999/999)\r\n");
      }
    }

    else if (current_mode == UART_MODE_SERVO) {
      bool angle_changed = false;
      if (received_byte == 'h') {
        UART_SendMessage("\r\n--- Exited Servo Mode ---\r\n");
        UART_CONTROL_init();
      } else if (received_byte == 'a') {
        current_servo_angle =
            (current_servo_angle >= 5) ? current_servo_angle - 5 : 0;
        angle_changed = true;
      } else if (received_byte == 'd') {
        current_servo_angle =
            (current_servo_angle <= 175) ? current_servo_angle + 5 : 180;
        angle_changed = true;
      } else if (received_byte == '1') {
        current_servo_angle = 0;
        angle_changed = true;
      } else if (received_byte == '2') {
        current_servo_angle = 45;
        angle_changed = true;
      } else if (received_byte == '3') {
        current_servo_angle = 90;
        angle_changed = true;
      }

      else if (received_byte == '4') {
        current_servo_angle = 135;
        angle_changed = true;
      }

      else if (received_byte == '5') {
        current_servo_angle = 180;
        angle_changed = true;
      }

      if (angle_changed) {
        Servo_SetAngle(current_servo_angle);
        char angle_buf[64];
        snprintf(angle_buf, sizeof(angle_buf), "\rCurrent Angle: %3d degrees",
                 current_servo_angle);
        UART_SendMessage(angle_buf);
      }
    }

    else if (current_mode == UART_MODE_STEPPER) {
      int16_t target_angle = current_stepper_angle;
      bool angle_changed = false;

      // head back to menu
      if (received_byte == 'h') {
        stepper_stop();
        UART_SendMessage("\r\n--- Exited Stepper Mode ---\r\n");
        UART_CONTROL_init();
      } else if (received_byte == 'a') {
        target_angle = current_stepper_angle - 5;
        if (target_angle < 0)
          target_angle = 0;
        angle_changed = true;
      } else if (received_byte == 'd') {
        target_angle = current_stepper_angle + 5;
        if (target_angle > 360)
          target_angle = 360;
        angle_changed = true;
      } else if (received_byte == '1') {
        target_angle = 0;
        angle_changed = true;
      } else if (received_byte == '2') {
        target_angle = 180;
        angle_changed = true;
      } else if (received_byte == '3') {
        target_angle = 360;
        angle_changed = true;
      }

      if (angle_changed && target_angle != current_stepper_angle) {
        int16_t diff = target_angle - current_stepper_angle;
        stepper_move_degrees((float)diff, 400);
        current_stepper_angle = target_angle;

        char angle_buf[64];
        snprintf(angle_buf, sizeof(angle_buf), "\rCurrent Angle:  %3d degrees",
                 current_stepper_angle);
        UART_SendMessage(angle_buf);
      }
    }

    else if (current_mode == UART_MODE_SPEAKER) {
      if (received_byte == 'h') {
        HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_2);
        UART_SendMessage("\r\n--- Exited Speaker Test Mode ---\r\n");
        UART_CONTROL_init();
      } else if (received_byte == '1') {
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 500);
        HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
        UART_SendMessage("\rPlaying 1 kHz tone... (Press '0' to stop)\r\n");
      } else if (received_byte == '2') {
        UART_SendMessage("\rPlaying 100ms beep...\r\n");
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 500);
        HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
        HAL_Delay(100);
        HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_2);
      } else if (received_byte == '0') {
        HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_2);
        UART_SendMessage("\rTone stopped.\r\n");
      }
    }
  }

  // Periodic sensor print
  static uint32_t last_print_time = 0;

  // if adding modes you need to add it here.
  if ((current_mode == UART_MODE_VOLTAGE || current_mode == UART_MODE_BOTH ||
       current_mode == UART_MODE_NORMALIZE) &&
      sensor_test_active && (HAL_GetTick() - last_print_time >= 250)) {
    last_print_time = HAL_GetTick();

    // this an array to temp store the characters to be sent out.
    char buffer[512];

    // set the buffer length to 0 to start.
    int len = 0;

    // Move cursor up lines if not the first print to keep the display static on
    // rows
    if (!first_print) {
      // Both Mode and Normalize Mode print 9 lines, Voltage Mode prints 8 lines
      if (current_mode == UART_MODE_BOTH ||
          current_mode == UART_MODE_NORMALIZE) {
        len += snprintf(buffer + len, sizeof(buffer) - len, "\x1B[9A");
      }

      else {
        len += snprintf(buffer + len, sizeof(buffer) - len, "\x1B[8A");
      }
    }
    if (first_print) {
      for (int i = 0; i < 8; i++) {
        min[i] = 4095; // set each channel to the value of 3.3V which is a white
                       // surface.
        max[i] =
            0; // set each channel to the value of 0V which is a black surface.
        filtered_adc[i] = 0;
      }
    }
    first_print = false;

    if (current_mode == UART_MODE_BOTH) {
      const char *state_str = "UNKNOWN";
      switch (Robot_GetState()) {
      case robot_idle:
        state_str = "IDLE";
        break;
      case robot_forward:
        state_str = "FORWARD";
        break;
      case robot_reverse:
        state_str = "REVERSE";
        break;
      case robot_left:
        state_str = "LEFT TURN";
        break;
      case robot_right:
        state_str = "RIGHT TURN";
        break;
      case robot_fault:
        state_str = "FAULT";
        break;
      case robot_auto:
        state_str = "AUTO";
        break;
      }
      int percent = (robot_speed * 100) / 999;
      len += snprintf(buffer + len, sizeof(buffer) - len,
                      "State: %-10s | Speed: %d%% PWM (%d/999) | Enc: L=%ld, "
                      "R=%ld         \r\n",
                      state_str, percent, robot_speed, Encoder_GetLeftTotal(),
                      Encoder_GetRightTotal());
    }

    uint8_t black_count = 0;

    // Iterate through all 8 channels of the ADC.
    for (uint8_t ch = 0; ch < 8; ch++) { // where ch is the iterating variable.

      // determine the raw ADC values
      uint16_t raw =
          MCP3208_ReadChannel(&hspi1, ADC_CS_GPIO_Port, ADC_CS_Pin, ch);

      // detection if theres any errors.
      if (raw == MCP3208_ERROR_VALUE) {
        len += snprintf(buffer + len, sizeof(buffer) - len,
                        "CH%d: ERR                     \r\n", ch);
      }

      // if no errors we can find the max and min
      else {
        // Apply Exponential Moving Average (EMA) filter to raw ADC
        if (filtered_adc[ch] == 0) {
          filtered_adc[ch] = raw;
        } else {
          float alpha = 0.3f;
          filtered_adc[ch] =
              (uint16_t)(alpha * raw + (1.0f - alpha) * filtered_adc[ch]);
        }
        uint16_t filtered_val = filtered_adc[ch];

        if (filtered_val < min[ch]) {
          min[ch] = filtered_val;
        }

        if (filtered_val > max[ch]) {
          max[ch] = filtered_val;
        }

        // for normalized mode, we dont print the actual voltage values.
        if (current_mode == UART_MODE_NORMALIZE) {
          // thresholds to classify color directly using filtered raw ADC
          const char *color = "Unknown"; // ptr to color.
          if (filtered_val < BLACK_THRESHOLD) {
            color = "Brown";
          } else {
            color = "Black";
            black_count++;
          }

          len += snprintf(buffer + len, sizeof(buffer) - len,
                          "CH%d: ADC: %4u | Color: %-6s                  \r\n",
                          ch, filtered_val, color);
        }
        // after we calculate the max and min values we can calculate the actual
        // voltage values.
        // if not normalized mode then we just calculate the voltage normally.
        else {
          // convert the raw ADC values from 16 bit to 32 bit.
          uint32_t actual_voltage = ((uint32_t)filtered_val * 3300) / 4095;
          if (current_mode == UART_MODE_VOLTAGE) {

            len += snprintf(
                buffer + len, sizeof(buffer) - len,
                "CH%d: %lu.%02luV | ADC: %4u | min: %u , max: %u  \r\n", ch,
                actual_voltage / 1000, (actual_voltage % 1000) / 10,
                filtered_val, min[ch], max[ch]);
          } else {
            len += snprintf(buffer + len, sizeof(buffer) - len,
                            "CH%d: %lu.%02luV | ADC: %4u                       "
                            "        \r\n",
                            ch, actual_voltage / 1000,
                            (actual_voltage % 1000) / 10, filtered_val);
          }
        }
      }
    }

    if (current_mode == UART_MODE_NORMALIZE) {
      len += snprintf(buffer + len, sizeof(buffer) - len,
                      "Line Detected: %s (Black Count: %d)          \r\n",
                      (black_count >= 3) ? "YES" : "NO ", black_count);
    }

    UART_SendMessage(buffer);
  }
}

// this function checks if we have timeouted or not.
void UART_CONTROL_check_timeout(void) {
  // If the robot is not idle, fault, or auto, check for communication timeout
  if (Robot_GetState() != robot_idle && Robot_GetState() != robot_fault &&
      Robot_GetState() != robot_auto) {

    // check if its been too long since we last got a command.
    if (HAL_GetTick() - last_command_time > 2000) {
      Robot_SetState(robot_fault);
      UART_SendMessage("TIMEOUT - ROBOT FAULT\r\n");
    }
  }
}

UART_ControlMode UART_CONTROL_GetMode(void) { return current_mode; }