#include "esp_timer.h"
#include "libmcu/timer.h"

#if !defined(TIMER_MAX)
#define TIMER_MAX			2
#endif

struct timer {
	struct timer_api api;

	timer_callback_t callback;
	void *arg;
	esp_timer_handle_t handle;
	bool periodic;
};

static void callback_wrapper(void *arg)
{
	struct timer *p = (struct timer *)arg;

	if (p && p->callback) {
		(*p->callback)(p, p->arg);
	}
}

static int start_timer(struct timer *self, uint32_t timeout_ms)
{
	uint64_t usec = timeout_ms * 1000;
	int err;

	if (self->periodic) {
		err = esp_timer_start_periodic(self->handle, usec);
	} else {
		err = esp_timer_start_once(self->handle, usec);
	}

	return err;
}

static int restart_timer(struct timer *self, uint32_t timeout_ms)
{
	uint64_t usec = timeout_ms * 1000;
	return esp_timer_restart(self->handle, usec);
}

static int stop_timer(struct timer *self)
{
	return esp_timer_stop(self->handle);
}

static int enable_timer(struct timer *self)
{
	return 0;
}

static int disable_timer(struct timer *self)
{
	return 0;
}

struct timer *timer_create(bool periodic, timer_callback_t callback, void *arg)
{
	static struct timer timers[TIMER_MAX];
	static int cnt;

	if (cnt >= TIMER_MAX) {
		return NULL;
	}

	struct timer *timer = &timers[cnt++];

	*timer = (struct timer) {
		.api = {
			.enable = enable_timer,
			.disable = disable_timer,
			.start = start_timer,
			.restart = restart_timer,
			.stop = stop_timer,
		},

		.callback = callback,
		.arg = arg,
		.periodic = periodic,
	};

	esp_timer_create_args_t param = {
		.callback = callback_wrapper,
		.arg = (void *)timer,
	};

	if (esp_timer_create(&param, &timer->handle) != ESP_OK) {
		return NULL;
	}

	return timer;
}

int timer_delete(struct timer *self)
{
	ESP_ERROR_CHECK(esp_timer_delete(self->handle));
	return 0;
}
