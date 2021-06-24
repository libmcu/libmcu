# button

## Overview
The debouncer that I implemented here learned from [Elliot Williams's Debounce Your Noisy Buttons article](https://hackaday.com/2015/12/10/embed-with-elliot-debounce-your-noisy-buttons-part-ii/).

## Integration Guide
### Define Parameters
* `BUTTON_MAX`
  - The maximum number of buttons. The default is 1.
* `BUTTON_SAMPLING_PERIOD_MS`
  - The sampling period. The default is 10 milli seconds.
* `BUTTON_MIN_PRESS_TIME_MS`
  - The default is 60 milli seconds.
 
### Initialize GPIO to be used for button
This is platform specific, somthing like in case of NRF5:

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
static void btn_pressed(const struct button_data *btn, void *context) {
	debug("button#1 pressed %d, %x", btn->time_pressed, btn->history);
}
static void btn_released(const struct button_data *btn, void *context) {
	debug("button#1 released %d, %x", btn->time_released, btn->history);
}

void register_buttons(void) {
	button_handlers_t mybtn = {
		.pressed = btn_pressed,
		.released = btn_released,
	};
	button_register(&mybtn, testbtn_get_state);
}
```

### And scan

```c
button_poll(NULL);
```
