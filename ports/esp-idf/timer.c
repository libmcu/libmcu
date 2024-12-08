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

struct apptmr {
	struct apptmr_api api;

	apptmr_callback_t callback;
	void *arg;
	esp_timer_handle_t handle;
	bool periodic;
};

static struct apptmr *new_apptmr(struct apptmr *pool, size_t n)
{
	struct apptmr empty = { 0, };

	for (size_t i = 0; i < n; i++) {
		if (memcmp(&empty, &pool[i], sizeof(empty)) == 0) {
			return &pool[i];
		}
	}

	return NULL;
}

static void free_apptmr(struct apptmr *apptmr)
{
	memset(apptmr, 0, sizeof(*apptmr));
}

static void callback_wrapper(void *arg)
{
	struct apptmr *p = (struct apptmr *)arg;

	if (p && p->callback) {
		(*p->callback)(p, p->arg);
	}
}

static int start_apptmr(struct apptmr *self, uint32_t timeout_ms)
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

static int restart_apptmr(struct apptmr *self, uint32_t timeout_ms)
{
	uint64_t usec = timeout_ms * 1000;
	return esp_timer_restart(self->handle, usec);
}

static int stop_apptmr(struct apptmr *self)
{
	return esp_timer_stop(self->handle);
}

static int enable_apptmr(struct apptmr *self)
{
	return 0;
}

static int disable_apptmr(struct apptmr *self)
{
	return 0;
}

struct apptmr *apptmr_create(bool periodic, apptmr_callback_t cb, void *cb_ctx)
{
	static struct apptmr apptmrs[TIMER_MAX];
	struct apptmr *apptmr = new_apptmr(apptmrs, TIMER_MAX);

	if (apptmr) {
		*apptmr = (struct apptmr) {
			.api = {
				.enable = enable_apptmr,
				.disable = disable_apptmr,
				.start = start_apptmr,
				.restart = restart_apptmr,
				.stop = stop_apptmr,
			},

			.callback = cb,
			.arg = cb_ctx,
			.periodic = periodic,
		};

		esp_timer_create_args_t param = {
			.callback = callback_wrapper,
			.arg = (void *)apptmr,
		};

		if (esp_timer_create(&param, &apptmr->handle) != ESP_OK) {
			return NULL;
		}
	}

	return apptmr;
}

int apptmr_delete(struct apptmr *self)
{
	ESP_ERROR_CHECK(esp_timer_delete(self->handle));
	free_apptmr(self);
	return 0;
}
