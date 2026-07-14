#include <blinky.h>
#include <button.h>
#include <main.h>

// creating the functions
void Blinky_Init(void) {
  // Make it noticeable when reset is clicked by doing a startup blink sequence:
  // Blink the LED 10 times (100ms ON, 100ms OFF)
  for (int i = 0; i < 10; i++) {
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
    HAL_Delay(100);
  }
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