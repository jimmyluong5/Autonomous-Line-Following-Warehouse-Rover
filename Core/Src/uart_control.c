#include "robot.h"
#include <main.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uart_control.h>

extern UART_HandleTypeDef hcom_uart[];

static uint32_t last_command_time = 0;

static void UART_SendMessage(const char *message) {
  HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)message, strlen(message),
                    HAL_MAX_DELAY);
}

static uint32_t led_blink_start_time = 0;
static bool led_is_blinking = false;

void UART_CONTROL_update(void) {
  uint8_t received_byte;

  // Handle non-blocking keypress LED turn-off after 50ms
  if (led_is_blinking && (HAL_GetTick() - led_blink_start_time >= 50)) {
    // Only turn off if the robot is currently idle or in fault state (other states keep it ON)
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
    __HAL_UART_CLEAR_FLAG(&hcom_uart[COM1], UART_CLEAR_NEF | UART_CLEAR_FEF | UART_CLEAR_PEF);
  }

  // Check if one keyboard character was received (non-blocking)
  if (HAL_UART_Receive(&hcom_uart[COM1], &received_byte, 1, 0) == HAL_OK) {
    // Turn on the LED to indicate keypress
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
    led_blink_start_time = HAL_GetTick();
    led_is_blinking = true;

    // Update the last command timestamp
    last_command_time = HAL_GetTick();

    if (received_byte == 'w') {
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
    }
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