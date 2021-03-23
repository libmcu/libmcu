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
 
### Initialize the module
A delay function for the sampling period and a time function to calculate
elapsed time should be provided when initializing:
`button_init(get_time_ms, sleep_ms)`.

And `button_init()` internally calls `button_hw_init()` to configure hardware.
Implement it something like:

```c
#define BUTTON1_BIT				GPIO_Pin_14
#define BUTTON1_PIN				GPIO_NUM_14
#define BUTTON2_BIT				GPIO_Pin_12
#define BUTTON2_PIN				GPIO_NUM_12

static void enable_button_interrupt(void)
{
	gpio_set_intr_type(BUTTON1_PIN, GPIO_INTR_NEGEDGE);
	gpio_set_intr_type(BUTTON2_PIN, GPIO_INTR_NEGEDGE);
}

static void disable_button_interrupt(void)
{
	gpio_set_intr_type(BUTTON1_PIN, GPIO_INTR_DISABLE);
	gpio_set_intr_type(BUTTON2_PIN, GPIO_INTR_DISABLE);
}

static void button_task(void *context)
{
	button_poll(context);
	enable_button_interrupt();
}

static void isr_button(void *arg)
{
	jobpool_schedule(button_task, arg);
	disable_button_interrupt();
}

int switch1_get_state(void)
{
	return !gpio_get_level(BUTTON1_PIN);
}

int switch2_get_state(void)
{
	return !gpio_get_level(BUTTON2_PIN);
}

void button_hw_init(void)
{
	gpio_config_t io_conf = {
		.pin_bit_mask = BUTTON1_BIT | BUTTON2_BIT,
		.intr_type = GPIO_INTR_NEGEDGE,
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
	};
	gpio_config(&io_conf);

	gpio_install_isr_service(0);
	gpio_isr_handler_add(BUTTON1_PIN, isr_button, (void *)BUTTON1_PIN);
	gpio_isr_handler_add(BUTTON2_PIN, isr_button, (void *)BUTTON2_PIN);
}
```

### Register buttons

```c
static void button1_pressed(const struct button_data *btn, void *context)
{
	debug("button#1 pressed %d, %x", btn->time_pressed, btn->history);
}

static void button1_released(const struct button_data *btn, void *context)
{
	debug("button#1 released %d, %x", btn->time_released, btn->history);
}

static void button2_pressed(const struct button_data *btn, void *context)
{
	debug("button#2 pressed %d, %x", btn->time_pressed, btn->history);
}

static void button2_released(const struct button_data *btn, void *context)
{
	debug("button#2 released %d, %x", btn->time_released, btn->history);
}

void register_buttons(void)
{
	button_handlers_t button1 = {
		.pressed = button1_pressed,
		.released = button1_released,
	};
	button_handlers_t button2 = {
		.pressed = button2_pressed,
		.released = button2_released,
	};

	button_register(&button1, switch1_get_state);
	button_register(&button2, switch2_get_state);
}
```
