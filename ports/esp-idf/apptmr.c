/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/apptmr.h"
#include <string.h>
#include "esp_timer.h"

#if !defined(APPTMR_MAX)
#define APPTMR_MAX			8
#endif

#if !defined(APPTMR_INFO)
#define APPTMR_INFO(...)
#endif

struct apptmr {
	struct apptmr_api api;

	apptmr_callback_t callback;
	void *arg;
	esp_timer_handle_t handle;
	bool periodic;
};

static int count_created;

static struct apptmr *new_apptmr(struct apptmr *pool, size_t n)
{
	const struct apptmr empty = { 0, };

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

	apptmr_global_pre_timeout_hook(p);

	if (p && p->callback) {
		(*p->callback)(p, p->arg);
	}

	apptmr_global_post_timeout_hook(p);
}

static void trigger(struct apptmr *self)
{
	callback_wrapper(self);
}

static int start_apptmr(struct apptmr *self, const uint32_t timeout_ms)
{
	const uint64_t usec = timeout_ms * 1000;
	int err;

	if (self->periodic) {
		err = esp_timer_start_periodic(self->handle, usec);
	} else {
		err = esp_timer_start_once(self->handle, usec);
	}

	return err;
}

static int restart_apptmr(struct apptmr *self, const uint32_t timeout_ms)
{
	const uint64_t usec = timeout_ms * 1000;
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

int apptmr_cap(void)
{
	return APPTMR_MAX;
}

int apptmr_len(void)
{
	return count_created;
}

struct apptmr *apptmr_create(bool periodic, apptmr_callback_t cb, void *cb_ctx)
{
	static struct apptmr apptmrs[APPTMR_MAX];
	struct apptmr *apptmr = new_apptmr(apptmrs, APPTMR_MAX);

	if (apptmr) {
		*apptmr = (struct apptmr) {
			.api = {
				.enable = enable_apptmr,
				.disable = disable_apptmr,
				.start = start_apptmr,
				.restart = restart_apptmr,
				.stop = stop_apptmr,
				.trigger = trigger,
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

		count_created++;
		APPTMR_INFO("apptmr created(%d): %p\n", count_created, apptmr);
	}

	return apptmr;
}

int apptmr_delete(struct apptmr *self)
{
	ESP_ERROR_CHECK(esp_timer_delete(self->handle));
	free_apptmr(self);
	count_created--;
	APPTMR_INFO("apptmr deleted(%d): %p\n", count_created, self);
	return 0;
}
