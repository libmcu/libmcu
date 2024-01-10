#include "CppUTestExt/MockSupport.h"
#include "libmcu/gpio.h"
#include "bytearray.h"

struct gpio {
	struct gpio_api api;
};

static int enable_gpio(struct gpio *self) {
	return mock().actualCall("gpio_enable").withParameter("self", self)
		.returnIntValue();
}

static int disable_gpio(struct gpio *self) {
	return mock().actualCall("gpio_disable").withParameter("self", self)
		.returnIntValue();
}

static int set_gpio(struct gpio *self, int value) {
	return mock().actualCall("gpio_set")
		.withParameter("self", self)
		.withParameter("value", value)
		.returnIntValue();
}

static int get_gpio(struct gpio *self) {
	return mock().actualCall("gpio_get")
		.withParameter("self", self)
		.returnIntValue();
}

static int register_callback(struct gpio *self, gpio_callback_t cb, void *cb_ctx) {
	return mock().actualCall("gpio_register_callback")
		.withParameter("self", self)
		.withParameter("cb", cb)
		.withParameter("cb_ctx", cb_ctx)
		.returnIntValue();
}

struct gpio *gpio_create(uint16_t pin) {
	(void)pin;

	static struct gpio gpio = {
		.api = {
			.enable = enable_gpio,
			.disable = disable_gpio,
			.set = set_gpio,
			.get = get_gpio,
			.register_callback = register_callback,
		},
	};

	return &gpio;
}

void gpio_delete(struct gpio *self) {
	mock().actualCall(__func__).withParameter("self", self);
}
