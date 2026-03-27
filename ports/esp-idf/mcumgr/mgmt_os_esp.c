/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_system.h"

#include "os_mgmt/os_mgmt.h"
#include "os_mgmt/os_mgmt_impl.h"
#include "mgmt/mgmt.h"

static void reset_timer_cb(TimerHandle_t timer)
{
	(void)timer;
	esp_restart();
}

/* -------------------------------------------------------------------------
 * os_mgmt_impl_* — called directly by the mcumgr OS management core.
 * ---------------------------------------------------------------------- */

int os_mgmt_impl_reset(unsigned int delay_ms)
{
	TickType_t ticks = pdMS_TO_TICKS(delay_ms ? delay_ms : 1U);
	TimerHandle_t timer = xTimerCreate("mgmt_reset", ticks,
			pdFALSE, NULL, reset_timer_cb);
	if (!timer) {
		return MGMT_ERR_ENOMEM;
	}

	xTimerStart(timer, 0);
	return 0;
}

int os_mgmt_impl_task_info(int idx, struct os_mgmt_task_info *out_info)
{
	(void)idx;
	(void)out_info;
	return MGMT_ERR_ENOENT;
}

int os_mgmt_impl_datetime_info(char *datetime, size_t datetime_size)
{
	(void)datetime;
	(void)datetime_size;
	return MGMT_ERR_ENOTSUP;
}

int os_mgmt_impl_datetime_set(char *datetime)
{
	(void)datetime;
	return MGMT_ERR_ENOTSUP;
}
