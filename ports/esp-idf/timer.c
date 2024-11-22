/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/timer.h"
#include <string.h>
#include "esp_timer.h"

#if !defined(TIMER_MAX)
#define TIMER_MAX			8
#endif

struct timer {
	struct timer_api api;

	timer_callback_t callback;
	void *arg;
	esp_timer_handle_t handle;
	bool periodic;
};

static struct timer *new_timer(struct timer *pool, size_t n)
{
	struct timer empty = { 0, };

	for (size_t i = 0; i < n; i++) {
		if (memcmp(&empty, &pool[i], sizeof(empty)) == 0) {
			return &pool[i];
		}
	}

	return NULL;
}

static void free_timer(struct timer *timer)
{
	memset(timer, 0, sizeof(*timer));
}

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
	struct timer *timer = new_timer(timers, TIMER_MAX);

	if (timer) {
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
	}

	return timer;
}

int timer_delete(struct timer *self)
{
	ESP_ERROR_CHECK(esp_timer_delete(self->handle));
	free_timer(self);
	return 0;
}
