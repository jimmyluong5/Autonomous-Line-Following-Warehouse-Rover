#include "robot.h"
#include <main.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uart_control.h>

extern UART_HandleTypeDef hcom_uart[];

static void UART_SendMessage(const char *message) {
  HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)message, strlen(message),
                    HAL_MAX_DELAY);
}

void UART_CONTROL_update(void) {
  uint8_t received_byte;

  // Check if one keyboard character was received
  if (HAL_UART_Receive(&hcom_uart[COM1], &received_byte, 1, 0) == HAL_OK) {
    // Blink the LED to indicate a keypress
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);

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