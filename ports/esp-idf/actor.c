/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/actor_overrides.h"
#include "libmcu/actor_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#if !defined(ACTOR_TIMER_INTERVAL_MS)
#define ACTOR_TIMER_INTERVAL_MS		50UL
#endif

static portMUX_TYPE actor_spinlock = portMUX_INITIALIZER_UNLOCKED;

static void on_timeout(void *arg)
{
	(void)arg;

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
	const esp_timer_create_args_t timer_args = {
		.callback = on_timeout,
	};
	esp_timer_handle_t actor_timer;
	esp_timer_create(&timer_args, &actor_timer);
	esp_timer_start_periodic(actor_timer,
			  ACTOR_TIMER_INTERVAL_MS * 1000/*usec*/);

	return 0;
}

void actor_lock(void *lock_handle)
{
	taskENTER_CRITICAL(&actor_spinlock);
}

void actor_unlock(void *lock_handle)
{
	taskEXIT_CRITICAL(&actor_spinlock);
}
