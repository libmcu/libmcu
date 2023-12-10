/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/actor_overrides.h"
#include "libmcu/actor_timer.h"
#include "libmcu/assert.h"

#include <errno.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#if !defined(ACTOR_TIMER_INTERVAL_MS)
#define ACTOR_TIMER_INTERVAL_MS		50UL
#endif

static unsigned long intctx;

static bool in_interrupt(void)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
	return xPortIsInsideInterrupt();
#pragma GCC diagnostic pop
}

static void on_timeout(TimerHandle_t xTimer)
{
	(void)xTimer;

	static TickType_t previous_tick_count;
	TickType_t current_tick_count = xTaskGetTickCount();
	TickType_t elapsed = current_tick_count - previous_tick_count;
	uint32_t elapsed_ms = elapsed * 1000 / configTICK_RATE_HZ;

	if (elapsed_ms) {
		actor_timer_step(elapsed_ms);
	}

	previous_tick_count = current_tick_count;
}

void actor_timer_boot(void)
{
	TimerHandle_t actor_timer_handle = xTimerCreate("AO Timer",
			pdMS_TO_TICKS(AO_TIMER_SCAN_INTERVAL_MS), pdTRUE,
			(void *)0, on_timeout);
	assert(actor_timer_handle);

	BaseType_t rc = xTimerStart(actor_timer_handle, 0);
	assert(rc == pdPASS);
}

void actor_lock(void)
{
	if (in_interrupt()) {
		intctx = taskENTER_CRITICAL_FROM_ISR();
	} else {
		taskENTER_CRITICAL();
	}
}

void actor_unlock(void)
{
	if (in_interrupt()) {
		taskEXIT_CRITICAL_FROM_ISR(intctx);
	} else {
		taskEXIT_CRITICAL();
	}
}
