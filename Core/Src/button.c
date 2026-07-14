#include <button.h>
#include <main.h>
#include <stdbool.h>

#define debounce_ms 20U // U for unsigned integer

// non blocking button press detection including debouncing detection

bool button_press(void) {
  // making two states for the button
  // if the button is not pressed the state is high
  // if the button is pressed the state is low
  static GPIO_PinState last_state = GPIO_PIN_SET;
  static GPIO_PinState curr_state = GPIO_PIN_SET;

  // stores the last time the raw button state changed
  static uint32_t prev_time = 0U;

  // determine the raw state of the pin
  // the button is active low because it reads low when pressed
  GPIO_PinState raw_state = HAL_GPIO_ReadPin(Led_Button_GPIO_Port, Led_Button_Pin);

  // get the current time in milliseconds
  uint32_t curr_time = HAL_GetTick();

  // if the raw button state changed then the button may be bouncing
  // save the new raw state and restart the debounce timer
  if (raw_state != last_state) {
    last_state = raw_state;
    prev_time = curr_time;
  }

  // check if the raw state has stayed the same for the debounce time
  if ((curr_time - prev_time) >= debounce_ms) {
    // if the stable state is different from the raw state
    // accept the raw state as the new stable state
    if (curr_state != raw_state) {
      curr_state = raw_state;

      // return true only when the button changes to pressed
      // GPIO_PIN_RESET means pressed because the button is active low
      if (curr_state == GPIO_PIN_RESET) {
        return true;
      }
    }
  }

  // return false if the button is released
  // still bouncing
  // or being held down
  return false;
}