/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include <errno.h>

#include "libmcu/board.h"

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

#include "os_mgmt/os_mgmt.h"
#include "os_mgmt/os_mgmt_impl.h"
#include "mgmt/mgmt.h"

static void reset_timer_cb(TimerHandle_t timer)
{
	(void)timer;
	board_reboot();
}

/* -------------------------------------------------------------------------
 * os_mgmt_impl_* — called directly by the mcumgr OS management core.
 * ---------------------------------------------------------------------- */

int os_mgmt_impl_reset(unsigned int delay_ms)
{
	if (!delay_ms) {
		board_reboot();
		return 0;
	}

	TimerHandle_t timer = xTimerCreate("mgmt_reset",
			pdMS_TO_TICKS(delay_ms), pdFALSE, NULL, reset_timer_cb);
	if (!timer) {
		board_reboot();
		return 0;
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
