/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ao_overrides.h"
#include "libmcu/ao_timer.h"

#include <errno.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

static unsigned long intctx;
static unsigned long intctx_timer;

static bool in_interrupt(void)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
	return xPortIsInsideInterrupt();
#pragma GCC diagnostic pop
}

static void on_ao_timeout(TimerHandle_t xTimer)
{
	(void)xTimer;

	static TickType_t previous_tick_count;
	TickType_t current_tick_count = xTaskGetTickCount();
	TickType_t elapsed = current_tick_count - previous_tick_count;
	uint32_t elapsed_ms = elapsed * 1000 / configTICK_RATE_HZ;

	if (elapsed_ms) {
		ao_timer_step(elapsed_ms);
	}

	previous_tick_count = current_tick_count;
}

int ao_timer_init(void)
{
	ao_timer_reset();

	TimerHandle_t ao_timer_handle = xTimerCreate("AO Timer",
			pdMS_TO_TICKS(AO_TIMER_SCAN_INTERVAL_MS), pdTRUE,
			(void *)0, on_ao_timeout);

	if (ao_timer_handle == NULL) {
		return -ENOMEM;
	}

	if (xTimerStart(ao_timer_handle, 0) != pdPASS) {
		return -EFAULT;
	}

	return 0;
}

void ao_lock(void *lock_handle)
{
	(void)lock_handle;

	if (in_interrupt()) {
		intctx = taskENTER_CRITICAL_FROM_ISR();
	} else {
		taskENTER_CRITICAL();
	}
}

void ao_unlock(void *lock_handle)
{
	(void)lock_handle;

	if (in_interrupt()) {
		taskEXIT_CRITICAL_FROM_ISR(intctx);
	} else {
		taskEXIT_CRITICAL();
	}
}

void ao_timer_lock(void)
{
	if (in_interrupt()) {
		intctx_timer = taskENTER_CRITICAL_FROM_ISR();
	} else {
		taskENTER_CRITICAL();
	}
}

void ao_timer_unlock(void)
{
	if (in_interrupt()) {
		taskEXIT_CRITICAL_FROM_ISR(intctx_timer);
	} else {
		taskEXIT_CRITICAL();
	}
}

void ao_timer_lock_init(void)
{
	/* nothing to do */
}
