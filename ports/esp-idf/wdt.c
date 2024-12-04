/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/wdt.h"
#include <stdlib.h>
#include "esp_task_wdt.h"
#include "libmcu/board.h"

struct wdt {
	void *task_handle;
};

struct wdt_manager {
	wdt_timeout_cb_t cb;
	void *cb_ctx;
};

static struct wdt_manager mgr;

int wdt_feed(struct wdt *self)
{
	(void)self;
	int err = esp_task_wdt_reset();
	return err == ESP_OK ? 0 : -err;
}

struct wdt *wdt_new(void)
{
	struct wdt *self = (struct wdt *)malloc(sizeof(*self));

	if (self) {
		self->task_handle = board_get_current_thread();
	}
	
	int err = esp_task_wdt_add(NULL);

	return err == ESP_OK? self : NULL;
}

void wdt_delete(struct wdt *self)
{
	esp_task_wdt_delete(NULL);
	free(self);
}

int wdt_init(wdt_timeout_cb_t cb, void *cb_ctx)
{
	int err = 0;

	mgr.cb = cb;
	mgr.cb_ctx = cb_ctx;

#if !CONFIG_ESP_TASK_WDT_INIT
	esp_task_wdt_config_t twdt_config = {
		.timeout_ms = CONFIG_TASK_WDT_TIMEOUT_S,
		.idle_core_mask = (1 << CONFIG_FREERTOS_NUMBER_OF_CORES) - 1,
		.trigger_panic = false,
	};
	err = esp_task_wdt_init(&twdt_config);
	err = err == ESP_OK? 0 : -err;
#endif

	return err;
}

void wdt_deinit(void)
{
#if !CONFIG_ESP_TASK_WDT_INIT
	ESP_ERROR_CHECK(esp_task_wdt_deinit());
#endif
}
