#include "robot.h"
#include <MCP3208.h>
#include <main.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uart_control.h>

extern UART_HandleTypeDef hcom_uart[];
extern SPI_HandleTypeDef hspi1;

static uint32_t last_command_time = 0;
static bool sensor_test_active = false;
static bool first_print = true;
static UART_ControlMode current_mode = UART_MODE_MENU;

static void UART_SendMessage(const char *message) {
  HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)message, strlen(message),
                    HAL_MAX_DELAY);
}

static uint32_t led_blink_start_time = 0;
static bool led_is_blinking = false;

void UART_CONTROL_init(void) {
  current_mode = UART_MODE_MENU;
  sensor_test_active = false;

  char menu_buf[512];
  int percent = (robot_speed * 100) / 999;
  snprintf(menu_buf, sizeof(menu_buf),
           "\r\n==========================================\r\n"
           "       Warehouse Rover Control Menu\r\n"
           "==========================================\r\n"
           "Select Option/Mode:\r\n"
           " [m] - Motor Control Mode\r\n"
           " [v] - Voltage / IR Sensor Test Mode\r\n"
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

void UART_CONTROL_update(void) {
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
            "\r\n--- Motor Control Mode Active ---\r\n"
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
      } else if (received_byte == 'v') {
        current_mode = UART_MODE_VOLTAGE;
        sensor_test_active = true;
        first_print = true;
        UART_SendMessage("\r\n--- Sensor Test Mode Active (Press 'h' to return "
                         "to Main Menu) ---\r\n");
      } else if (received_byte == '1') {
        robot_speed = 250;
        UART_SendMessage("\r\nSpeed set to 25% PWM (250/999)\r\n");
        UART_CONTROL_init();
      } else if (received_byte == '2') {
        robot_speed = 500;
        UART_SendMessage("\r\nSpeed set to 50% PWM (500/999)\r\n");
        UART_CONTROL_init();
      } else if (received_byte == '3') {
        robot_speed = 750;
        UART_SendMessage("\r\nSpeed set to 75% PWM (750/999)\r\n");
        UART_CONTROL_init();
      } else if (received_byte == '4') {
        robot_speed = 999;
        UART_SendMessage("\r\nSpeed set to 100% PWM (999/999)\r\n");
        UART_CONTROL_init();
      } else {
        UART_CONTROL_init(); // Reprint menu on invalid key
      }
    } else if (current_mode == UART_MODE_MOTOR) {
      if (received_byte == 'h') {
        Robot_SetState(robot_idle);
        UART_SendMessage("\r\nExiting Motor Mode. Stopping Robot.\r\n");
        UART_CONTROL_init();
      } else if (received_byte == 'w') {
        Robot_SetState(robot_forward);
        UART_SendMessage("ROBOT FORWARD\r\n");
      } else if (received_byte == 's') {
        Robot_SetState(robot_reverse);
        UART_SendMessage("ROBOT REVERSE\r\n");
      } else if (received_byte == 'x') {
        Robot_SetState(robot_idle);
        UART_SendMessage("ROBOT STOPPED\r\n");
      } else if (received_byte == 'a') {
        Robot_SetState(robot_left);
        UART_SendMessage("ROBOT LEFT\r\n");
      } else if (received_byte == 'd') {
        Robot_SetState(robot_right);
        UART_SendMessage("ROBOT RIGHT\r\n");
      } else if (received_byte == 'f') {
        Robot_SetState(robot_fault);
        UART_SendMessage("ROBOT FAULT\r\n");
      } else if (received_byte == '1') {
        robot_speed = 250;
        Robot_SetState(Robot_GetState());
        UART_SendMessage("Speed set to 25% PWM (250/999)\r\n");
      } else if (received_byte == '2') {
        robot_speed = 500;
        Robot_SetState(Robot_GetState());
        UART_SendMessage("Speed set to 50% PWM (500/999)\r\n");
      } else if (received_byte == '3') {
        robot_speed = 750;
        Robot_SetState(Robot_GetState());
        UART_SendMessage("Speed set to 75% PWM (750/999)\r\n");
      } else if (received_byte == '4') {
        robot_speed = 999;
        Robot_SetState(Robot_GetState());
        UART_SendMessage("Speed set to 100% PWM (999/999)\r\n");
      }
    } else if (current_mode == UART_MODE_VOLTAGE) {
      if (received_byte == 'h' || received_byte == 'v' ||
          received_byte == 'x') {
        sensor_test_active = false;
        UART_SendMessage("\r\n--- Exited Sensor Test Mode ---\r\n");
        UART_CONTROL_init();
      }
    }
  }

  // Periodic sensor print
  static uint32_t last_print_time = 0;
  if (current_mode == UART_MODE_VOLTAGE && sensor_test_active &&
      (HAL_GetTick() - last_print_time >= 250)) {
    last_print_time = HAL_GetTick();
    char buffer[512];
    int len = 0;

    // Move cursor up 8 lines if not the first print to keep the display static
    // on rows
    if (!first_print) {
      len += snprintf(buffer + len, sizeof(buffer) - len, "\x1B[8A");
    }
    first_print = false;

    for (uint8_t ch = 0; ch < 8; ch++) {
      uint16_t raw =
          MCP3208_ReadChannel(&hspi1, ADC_CS_GPIO_Port, ADC_CS_Pin, ch);
      if (raw == MCP3208_ERROR_VALUE) {
        len += snprintf(buffer + len, sizeof(buffer) - len,
                        "CH%d: ERR                     \r\n", ch);
      } else {
        uint32_t mv = ((uint32_t)raw * 3300) / 4095;
        len += snprintf(buffer + len, sizeof(buffer) - len,
                        "CH%d: %lu.%02luV | ADC: %4u       \r\n", ch, mv / 1000,
                        (mv % 1000) / 10, raw);
      }
    }
    UART_SendMessage(buffer);
  }
}

void UART_CONTROL_check_timeout(void) {
  // If the robot is not idle or fault, check for communication timeout
  if (Robot_GetState() != robot_idle && Robot_GetState() != robot_fault) {
    if (HAL_GetTick() - last_command_time > 2000) {
      Robot_SetState(robot_fault);
      UART_SendMessage("TIMEOUT - ROBOT FAULT\r\n");
    }
  }
}

UART_ControlMode UART_CONTROL_GetMode(void) { return current_mode; }