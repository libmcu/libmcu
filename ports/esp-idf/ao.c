/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ao_overrides.h"
#include "libmcu/ao_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

static portMUX_TYPE ao_spinlock = portMUX_INITIALIZER_UNLOCKED;

static void on_ao_timeout(void *arg)
{
	(void)arg;

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

	const esp_timer_create_args_t timer_args = {
		.callback = on_ao_timeout,
	};
	esp_timer_handle_t ao_timer;
	esp_timer_create(&timer_args, &ao_timer);
	esp_timer_start_periodic(ao_timer,
			AO_TIMER_SCAN_INTERVAL_MS * 1000/*usec*/);

	return 0;
}

void ao_lock(void *lock_handle)
{
	taskENTER_CRITICAL(&ao_spinlock);
}

void ao_unlock(void *lock_handle)
{
	taskEXIT_CRITICAL(&ao_spinlock);
}

void ao_timer_lock(void)
{
	taskENTER_CRITICAL(&ao_spinlock);
}

void ao_timer_unlock(void)
{
	taskEXIT_CRITICAL(&ao_spinlock);
}

void ao_timer_lock_init(void)
{
	/* nothing to do */
}
