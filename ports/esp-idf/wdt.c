/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/wdt.h"

#include <stdlib.h>
#include <pthread.h>

#include "esp_task_wdt.h"
#include "libmcu/board.h"
#include "libmcu/list.h"
#include "libmcu/timext.h"

#define STACK_SIZE_BYTES	1024

#if !defined(MIN)
#define MIN(a, b)		((a) > (b)? (b) : (a))
#endif

struct wdt {
	void *task_handle;

	uint32_t period_ms; /* timeout period in milliseconds */
	uint32_t last_feed_ms; /* last feed time in milliseconds */

	wdt_timeout_cb_t cb;
	void *cb_ctx;

	struct list link;
};

struct wdt_manager {
	pthread_t thread;

	struct list list;

	wdt_timeout_cb_t cb;
	void *cb_ctx;

	uint32_t min_period_ms;
};

static struct wdt_manager m;

static bool is_timedout(struct wdt *wdt, const uint32_t now)
{
	return (now - wdt->last_feed_ms) >= wdt->period_ms;
}

static struct wdt *any_timeouts(struct wdt_manager *mgr, const uint32_t now)
{
	struct list *p;

	list_for_each(p, &mgr->list) {
		struct wdt *wdt = list_entry(p, struct wdt, link);
		if (is_timedout(wdt, now)) {
			return wdt;
		}
	}

	return NULL;
}

static void *wdt_task(void *e)
{
	struct wdt_manager *mgr = (struct wdt_manager *)e;
	ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

	while (1) {
		struct wdt *wdt = any_timeouts(mgr,
				board_get_time_since_boot_ms());

		if (!wdt) {
			esp_task_wdt_reset();
			sleep_ms(mgr->min_period_ms);
			continue;
		}

		if (wdt->cb) {
			(*wdt->cb)(wdt, wdt->cb_ctx);
		}
		if (mgr->cb) {
			(*mgr->cb)(wdt, mgr->cb_ctx);
		}

		sleep_ms(mgr->min_period_ms);
	}

	esp_task_wdt_delete(NULL);
	return 0;
}

int wdt_feed(struct wdt *self)
{
	self->last_feed_ms = board_get_time_since_boot_ms();
	return 0;
}

struct wdt *wdt_new(const uint32_t period_ms, wdt_timeout_cb_t cb, void *cb_ctx)
{
	struct wdt *wdt = (struct wdt *)malloc(sizeof(*wdt));

	if (wdt) {
		wdt->period_ms = period_ms;
		wdt->cb = cb;
		wdt->cb_ctx = cb_ctx;
		wdt->task_handle = board_get_current_thread();
		list_add(&wdt->link, &m.list);

		m.min_period_ms = MIN(m.min_period_ms, period_ms);
	}
	
	return wdt;
}

void wdt_delete(struct wdt *self)
{
	list_del(&self->link, &m.list);
	free(self);
}

int wdt_register_timeout_cb(wdt_timeout_cb_t cb, void *cb_ctx)
{
	m.cb = cb;
	m.cb_ctx = cb_ctx;

	return 0;
}

int wdt_init(void)
{
	m.min_period_ms = CONFIG_TASK_WDT_TIMEOUT_S * 1000;
	list_init(&m.list);

#if !CONFIG_ESP_TASK_WDT_INIT
	esp_task_wdt_config_t twdt_config = {
		.timeout_ms = CONFIG_TASK_WDT_TIMEOUT_S,
		.idle_core_mask = (1 << CONFIG_FREERTOS_NUMBER_OF_CORES) - 1,
		.trigger_panic = false,
	};
	int err = esp_task_wdt_init(&twdt_config);

	if (err != ESP_OK) {
		return -err;
	}
#endif

	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, STACK_SIZE_BYTES);

	return pthread_create(&m.thread, &attr, wdt_task, &m);
}

void wdt_deinit(void)
{
	pthread_exit(&m.thread);
#if !CONFIG_ESP_TASK_WDT_INIT
	ESP_ERROR_CHECK(esp_task_wdt_deinit());
#endif
}
