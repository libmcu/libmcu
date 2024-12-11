#include "CppUTestExt/MockSupport.h"
#include "libmcu/timer.h"

struct apptmr {
	struct apptmr_api api;
};

static int enable(struct apptmr *self) {
	return mock().actualCall("apptmr_enable")
		.withParameter("self", self)
		.returnIntValue();
}

static int disable(struct apptmr *self) {
	return mock().actualCall("apptmr_disable")
		.withParameter("self", self)
		.returnIntValue();
}

static int start(struct apptmr *self, uint32_t timeout_ms) {
	return mock().actualCall("apptmr_start")
		.withParameter("self", self)
		.withParameter("timeout_ms", timeout_ms)
		.returnIntValue();
}

static int restart(struct apptmr *self, uint32_t timeout_ms) {
	return mock().actualCall("apptmr_restart")
		.withParameter("self", self)
		.withParameter("timeout_ms", timeout_ms)
		.returnIntValue();
}

static int stop(struct apptmr *self) {
	return mock().actualCall("apptmr_stop")
		.withParameter("self", self)
		.returnIntValue();
}

struct apptmr *apptmr_create(bool periodic, apptmr_callback_t cb, void *cb_ctx)
{
	struct apptmr *self = new struct apptmr;
	self->api.enable = enable;
	self->api.disable = disable;
	self->api.start = start;
	self->api.restart = restart;
	self->api.stop = stop;
	return self;
}

int apptmr_delete(struct apptmr *self)
{
	delete self;
	return 0;
}
