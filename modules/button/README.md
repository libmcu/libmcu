# button

## Overview
The debouncer that I implemented here learned from [Elliot Williams's Debounce Your Noisy Buttons article](https://hackaday.com/2015/12/10/embed-with-elliot-debounce-your-noisy-buttons-part-ii/).

## Integration Guide
### Default Parameters
* `BUTTON_MAX`
  - The maximum number of buttons. The default is 1.
* `BUTTON_SAMPLING_INTERVAL_MS`
  - The sampling period. The default is 10 milliseconds.
* `BUTTON_MIN_PRESS_TIME_MS`
  - The default is 60 milliseconds.
* `BUTTON_REPEAT_DELAY_MS`
  - The repeat handler is called after the defined delay while button holding. The default is 300 milliseconds.
* `BUTTON_REPEAT_RATE_MS`
  - The repeat handler is called every BUTTON_REPEAT_RATE_MS while button holding. The default is 200 milliseconds.
* `BUTTON_CLICK_WINDOW_MS`
  - The click handler is called with the number of clicks when another click comes in the time window. The default is 500 milliseconds.
 
### Initialize GPIO to be used for button
This is platform specific, something like in case of NRF5:

```c
button_level_t testbtn_get_state(void *ctx) {
	return nrf_gpio_pin_read(USER_BUTTON)? BUTTON_LEVEL_HIGH : BUTTON_LEVEL_LOW;
}
void testbtn_hw_init(void) {
	nrf_gpio_cfg_input(USER_BUTTON, NRF_GPIO_PIN_PULLUP);
}
```

> Interrupt can be a trigger to scan button states rather than polling all the
> time wasting cpu resource.

### Create a button

```c
static void on_button_event(struct button *btn, const button_state_t event,
		const uint8_t clicks, void *ctx) {
	switch (event) {
	case BUTTON_STATE_CLICK:
        printf("%d click(s)\n", clicks);
		break;
	case BUTTON_STATE_PRESSED:
        printf("pressed\n");
		break;
	case BUTTON_STATE_RELEASED:
        printf("released\n");
		break;
	case BUTTON_STATE_HOLDING:
        printf("holding\n");
		break;
	}
}

int main(void) {
	struct button *btn = button_new(testbtn_get_state, 0, on_button_event, 0);
    button_enable(btn);

    while (1) {
        const uint32_t now = millis();
        button_step(btn, now);
    }

    button_disable(btn);
    button_delete(btn);
}
```

then registered handler will be called when button activity detected.

## Notes
- Every changes of button state will be notified to the registered handler.
  - For example, if the button is pressed and held, the handler will be called with BUTTON_STATE_PRESSED and BUTTON_STATE_HOLDING.
  - If the button is pressed, held and one more click is detected, the handler will be called 7 times in total, with BUTTON_STATE_PRESSED, BUTTON_STATE_HOLDING, BUTTON_STATE_RELEASED, BUTTON_STATE_CLICK with 1 click, BUTTON_STATE_PRESSED, BUTTON_STATE_RELEASE and BUTTON_STATE_CLICK with 2 clicks.
