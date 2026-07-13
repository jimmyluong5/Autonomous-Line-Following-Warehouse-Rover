#include <blinky.h>
#include <button.h>
#include <main.h>

// creating the functions
void Blinky_Init(void) {
  // these functions are found in the stm32g4xx_hal_gpio.c file
  // they are imported into blinky.h
  // we are setting the led to off initially
  // ld2 is just an arbitrary name for the pin
  // you can find the pin in the main.c file

  // start with the LED off.
  HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
}

// updates the state of led
// to be called every 500ms
bool Blinky_update(void) {
  if (button_press()) {
    HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
    // button pressed detected
    return true;
  }
  // no button press
  return false;
}