# button

## Overview
The debouncer that I implemented here learned from [Elliot Williams's Debounce Your Noisy Buttons article](https://hackaday.com/2015/12/10/embed-with-elliot-debounce-your-noisy-buttons-part-ii/).

## Integration Guide
### Define Parameters
* `BUTTON_MAX`
  - The maximum number of buttons. The default is 1.
* `BUTTON_SAMPLING_PERIOD_MS`
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
int testbtn_get_state(void) {
	return (int)nrf_gpio_pin_read(USER_BUTTON);
}
void testbtn_hw_init(void) {
	nrf_gpio_cfg_input(USER_BUTTON, NRF_GPIO_PIN_PULLUP);
}
```

> Interrupt can be a trigger to scan button states rather than polling all the
> time wasting cpu resource.

### Initialize the module
A time function to calculate elapsed time should be provided when initializing:
`button_init(get_time_ms)`. The prototype is `unsigned int get_time_ms(void)`.

As an example:

```c
unsigned int get_time_ms(void) {
	return xTaskGetTickCount() * 1000 / configTICK_RATE_HZ;
}
```

### Register buttons

```c
static void on_button_event(enum button_event event,
		const struct button_data *info, void *ctx) {
	switch (event) {
	case BUTTON_EVT_CLICK:
		debug("%d click(s)", info->click);
		break;
	case BUTTON_EVT_PRESSED:
		debug("pressed at %lu", info->time_pressed);
		break;
	case BUTTON_EVT_RELEASED:
		debug("released at %lu", info->time_released);
		break;
	case BUTTON_EVT_HOLDING:
		debug("holding at %lu", info->time_repeat);
		break;
	}
}

void register_buttons(void) {
	button_register(testbtn_get_state, on_button_event, 0);
}
```

### And scan

```c
button_step();
```

then registered handler will be called when button activity detected.
