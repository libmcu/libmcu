#include "CppUTestExt/MockSupport.h"
#include "libmcu/timer.h"

struct timer {
	struct timer_api api;
};

static int enable(struct timer *self) {
	return mock().actualCall("timer_enable")
		.withParameter("self", self)
		.returnIntValue();
}

static int disable(struct timer *self) {
	return mock().actualCall("timer_disable")
		.withParameter("self", self)
		.returnIntValue();
}

static int start(struct timer *self, uint32_t timeout_ms) {
	return mock().actualCall("timer_start")
		.withParameter("self", self)
		.withParameter("timeout_ms", timeout_ms)
		.returnIntValue();
}

static int restart(struct timer *self, uint32_t timeout_ms) {
	return mock().actualCall("timer_restart")
		.withParameter("self", self)
		.withParameter("timeout_ms", timeout_ms)
		.returnIntValue();
}

static int stop(struct timer *self) {
	return mock().actualCall("timer_stop")
		.withParameter("self", self)
		.returnIntValue();
}

struct timer *timer_create(bool periodic, timer_callback_t callback, void *arg)
{
	struct timer *self = new struct timer;
	self->api.enable = enable;
	self->api.disable = disable;
	self->api.start = start;
	self->api.restart = restart;
	self->api.stop = stop;
	return self;
}

int timer_delete(struct timer *self)
{
	delete self;
	return 0;
}
