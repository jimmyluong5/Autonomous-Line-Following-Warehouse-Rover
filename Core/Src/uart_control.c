#include <main.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uart_control.h>

extern UART_HandleTypeDef hcom_uart[];

void UART_CONTROL_update(void) {
  uint8_t received_byte;

  // Check if one character was received from keyboard input.
  // The timeout is 0 so this does not block the program execution.
  if (HAL_UART_Receive(&hcom_uart[COM1], &received_byte, 1, 0) == HAL_OK) {
    if (received_byte == 'w') {
      HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
      char message[] = "Forward Command \r\n";
      HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
    }
    else if (received_byte == 's') {
      HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
      char message[] = "Backward Command \r\n";
      HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
    }
    else if (received_byte == 'a') {
      HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
      char message[] = "Left Command \r\n";
      HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
    }
    else if (received_byte == 'd') {
      HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
      char message[] = "Right Command \r\n";
      HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
    }
  }
}