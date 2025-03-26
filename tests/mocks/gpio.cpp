#include "CppUTestExt/MockSupport.h"
#include "libmcu/gpio.h"
#include "bytearray.h"

struct lm_gpio {
	struct lm_gpio_api api;
};

static int enable_gpio(struct lm_gpio *self) {
	return mock().actualCall("lm_gpio_enable").withParameter("self", self)
		.returnIntValue();
}

static int disable_gpio(struct lm_gpio *self) {
	return mock().actualCall("lm_gpio_disable").withParameter("self", self)
		.returnIntValue();
}

static int set_gpio(struct lm_gpio *self, int value) {
	return mock().actualCall("lm_gpio_set")
		.withParameter("self", self)
		.withParameter("value", value)
		.returnIntValue();
}

static int get_gpio(struct lm_gpio *self) {
	return mock().actualCall("lm_gpio_get")
		.withParameter("self", self)
		.returnIntValue();
}

static int register_callback(struct lm_gpio *self, lm_gpio_callback_t cb, void *cb_ctx) {
	return mock().actualCall("lm_gpio_register_callback")
		.withParameter("self", self)
		.withParameter("cb", cb)
		.withParameter("cb_ctx", cb_ctx)
		.returnIntValue();
}

struct lm_gpio *lm_gpio_create(uint16_t pin) {
	(void)pin;

	static struct lm_gpio gpio = {
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

void lm_gpio_delete(struct lm_gpio *self) {
	mock().actualCall(__func__).withParameter("self", self);
}
