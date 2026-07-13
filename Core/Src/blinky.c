#include <blinky.h>
#include <main.h>

// creating the functions
void Blinky_Init(void) {
  // these functions are found in the stm32g4xx_hal_gpio.c file
  // they are imported into blinky.h
  // we are setting the led to off initially
  // ld2 is just an arbitrary name for the pin
  // you can find the pin in the main.c file
  HAL_GPIO_WritePin(LD2_GPIO_PORT, LD2_PIN, GPIO_PIN_RESET);
}

// updates the state of led
// to be called every 500ms
void Blinky_update(void) {
  HAL_GPIO_TogglePin(LD2_GPIO_PORT, LD2_PIN);
  HAL_Delay(250); // hal_delay is in milliseconds
}