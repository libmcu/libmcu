/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/wdt.h"

#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "libmcu/board.h"
#include "libmcu/list.h"
#include "libmcu/timext.h"

#include "esp_task_wdt.h"

#if !defined(WDT_STACK_SIZE_BYTES)
#define WDT_STACK_SIZE_BYTES	1024U
#endif

#define STACK_SIZE_BYTES	WDT_STACK_SIZE_BYTES

#if !defined(MIN)
#define MIN(a, b)		((a) > (b)? (b) : (a))
#endif

#if !defined(WDT_ERROR)
#define WDT_ERROR(...)
#endif

struct wdt {
	void *task_handle;

	const char *name;
	uint32_t period_ms; /* timeout period in milliseconds */
	uint32_t last_feed_ms; /* last feed time in milliseconds */

	wdt_timeout_cb_t cb;
	void *cb_ctx;

	struct list link;

	bool enabled;
};

struct wdt_manager {
	pthread_t thread;
	pthread_mutex_t mutex;

	struct list list;

	wdt_timeout_cb_t cb;
	void *cb_ctx;

	wdt_periodic_cb_t periodic_cb;
	void *periodic_cb_ctx;

	uint32_t min_period_ms;
	bool threaded;
};

static struct wdt_manager m;

static uint32_t get_deadline_ms(const struct wdt *wdt)
{
	return wdt->last_feed_ms + wdt->period_ms;
}

static uint32_t get_time_until_deadline_ms(const struct wdt *wdt, uint32_t now)
{
	const uint32_t deadline_ms = get_deadline_ms(wdt);
	const uint32_t diff = deadline_ms - now;

	if (diff > wdt->period_ms) {
		return 0;
	}

	return diff;
}

static bool is_timedout(struct wdt *wdt, uint32_t now)
{
	return wdt->enabled && get_time_until_deadline_ms(wdt, now) == 0;
}

static void feed_wdt(struct wdt *self, uint32_t now)
{
	self->last_feed_ms = now;
}

static struct wdt *any_timeouts(struct wdt_manager *mgr, uint32_t now,
		uint32_t *next_deadline_ms)
{
	struct list *p;
	struct list *n;

	if (next_deadline_ms) {
		*next_deadline_ms = mgr->min_period_ms;
	}

	pthread_mutex_lock(&mgr->mutex);
	list_for_each_safe(p, n, &mgr->list) {
		struct wdt *wdt = list_entry(p, struct wdt, link);
		if (is_timedout(wdt, now)) {
			pthread_mutex_unlock(&mgr->mutex);
			return wdt;
		}
		if (next_deadline_ms) {
			const uint32_t t = get_time_until_deadline_ms(wdt, now);
			*next_deadline_ms = MIN(*next_deadline_ms, t);
		}
	}
	pthread_mutex_unlock(&mgr->mutex);

	return NULL;
}

static int process_timeouts(struct wdt_manager *mgr, uint32_t *next_deadline_ms)
{
	if (mgr->periodic_cb) {
		(*mgr->periodic_cb)(mgr->periodic_cb_ctx);
	}

	struct wdt *wdt = any_timeouts(mgr,
			board_get_time_since_boot_ms(), next_deadline_ms);

	if (wdt) {
		WDT_ERROR("wdt \"%s\" timed out", wdt->name);

		if (wdt->cb) {
			(*wdt->cb)(wdt, wdt->cb_ctx);
		}
		if (mgr->cb) {
			(*mgr->cb)(wdt, mgr->cb_ctx);
		}

		return -ETIMEDOUT;
	}

	esp_task_wdt_reset();

	return 0;
}

static void *wdt_task(void *e)
{
	struct wdt_manager *mgr = (struct wdt_manager *)e;

	while (1) {
		uint32_t next_deadline_ms;
		process_timeouts(mgr, &next_deadline_ms);
		sleep_ms(next_deadline_ms);
	}

	return 0;
}

int wdt_step(uint32_t *next_deadline_ms)
{
	return process_timeouts(&m, next_deadline_ms);
}

int wdt_feed(struct wdt *self)
{
	const uint32_t now = board_get_time_since_boot_ms();
	if (is_timedout(self, now)) {
		WDT_ERROR("wdt \"%s\" already timed out", self->name);
		return -ETIMEDOUT;
	}
	feed_wdt(self, now);
	return 0;
}

bool wdt_is_enabled(const struct wdt *self)
{
	return self->enabled;
}

int wdt_enable(struct wdt *self)
{
	feed_wdt(self, board_get_time_since_boot_ms());
	self->enabled = true;
	return 0;
}

int wdt_disable(struct wdt *self)
{
	self->enabled = false;
	return 0;
}

struct wdt *wdt_new(const char *name, const uint32_t period_ms,
		wdt_timeout_cb_t cb, void *cb_ctx)
{
	struct wdt *wdt = (struct wdt *)malloc(sizeof(*wdt));

	if (wdt) {
		*wdt = (struct wdt) {
			.name = name,
			.period_ms = period_ms,
			.cb = cb,
			.cb_ctx = cb_ctx,
			.task_handle = board_get_current_thread(),
		};

		pthread_mutex_lock(&m.mutex);
		list_add(&wdt->link, &m.list);
		m.min_period_ms = MIN(m.min_period_ms, period_ms);
		pthread_mutex_unlock(&m.mutex);
	}
	
	return wdt;
}

void wdt_delete(struct wdt *self)
{
	pthread_mutex_lock(&m.mutex);
	list_del(&self->link, &m.list);
	pthread_mutex_unlock(&m.mutex);

	free(self);
}

int wdt_register_timeout_cb(wdt_timeout_cb_t cb, void *cb_ctx)
{
	pthread_mutex_lock(&m.mutex);
	m.cb = cb;
	m.cb_ctx = cb_ctx;
	pthread_mutex_unlock(&m.mutex);

	return 0;
}

uint32_t wdt_get_period(const struct wdt *self)
{
	return self->period_ms;
}

uint32_t wdt_get_time_since_last_feed(const struct wdt *self)
{
	return board_get_time_since_boot_ms() - self->last_feed_ms;
}

const char *wdt_name(const struct wdt *self)
{
	return self->name;
}

void wdt_foreach(wdt_foreach_cb_t cb, void *cb_ctx)
{
	struct list *p;
	struct list *n;

	list_for_each_safe(p, n, &m.list) {
		struct wdt *wdt = list_entry(p, struct wdt, link);
		(*cb)(wdt, cb_ctx);
	}
}

int wdt_start(void)
{
	ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
	return 0;
}

void wdt_stop(void)
{
	esp_task_wdt_delete(NULL);
}

int wdt_init(wdt_periodic_cb_t cb, void *cb_ctx, bool threaded)
{
	int err = 0;

	m.min_period_ms = (CONFIG_TASK_WDT_TIMEOUT_S * 1000) / 2;
	m.periodic_cb = cb;
	m.periodic_cb_ctx = cb_ctx;
	m.threaded = threaded;
	list_init(&m.list);
	pthread_mutex_init(&m.mutex, NULL);

#if !CONFIG_ESP_TASK_WDT_INIT
	esp_task_wdt_config_t twdt_config = {
		.timeout_ms = (CONFIG_TASK_WDT_TIMEOUT_S * 1000) / 2,
		.idle_core_mask = (1 << CONFIG_FREERTOS_NUMBER_OF_CORES) - 1,
		.trigger_panic = false,
	};
	err = esp_task_wdt_init(&twdt_config);

	if (err != ESP_OK) {
		return -err;
	}
#endif
	if (threaded) {
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, STACK_SIZE_BYTES);
		err = pthread_create(&m.thread, &attr, wdt_task, &m);
		pthread_attr_destroy(&attr);
	}

	return err;
}

void wdt_deinit(void)
{
	if (m.threaded) {
		pthread_cancel(m.thread);
	}
	pthread_mutex_destroy(&m.mutex);
#if !CONFIG_ESP_TASK_WDT_INIT
	ESP_ERROR_CHECK(esp_task_wdt_deinit());
#endif
}
