#include "CppUTestExt/MockSupport.h"
#include "libmcu/apptmr.h"

struct apptmr {
	struct apptmr_api api;
	apptmr_callback_t cb;
	void *cb_ctx;
};

static int enable(struct apptmr *self) {
	return 0;
}

static int disable(struct apptmr *self) {
	return 0;
}

static int start(struct apptmr *self, uint32_t timeout_ms) {
	return 0;
}

static int restart(struct apptmr *self, uint32_t timeout_ms) {
	return 0;
}

static int stop(struct apptmr *self) {
	return 0;
}

static void trigger(struct apptmr *self)
{
	if (self->cb) {
		(*self->cb)(self, self->cb_ctx);
	}
}

struct apptmr *apptmr_create(bool periodic, apptmr_callback_t cb, void *cb_ctx)
{
	struct apptmr *self = new struct apptmr;
	self->api.enable = enable;
	self->api.disable = disable;
	self->api.start = start;
	self->api.restart = restart;
	self->api.stop = stop;
	self->api.trigger = trigger;
	self->cb = cb;
	self->cb_ctx = cb_ctx;
	apptmr_create_hook(self);
	return self;
}

int apptmr_delete(struct apptmr *self)
{
	delete self;
	return 0;
}
