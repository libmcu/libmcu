/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "apptimer_spy.h"
#include <stdlib.h>
#include "esp_timer.h"

esp_err_t esp_timer_init(void)
{
	return ESP_OK;
}

esp_err_t esp_timer_deinit(void)
{
	return ESP_OK;
}

esp_err_t esp_timer_create(const esp_timer_create_args_t *args,
                           esp_timer_handle_t *out_handle)
{
	uint32_t *state = (uint32_t *)calloc(1, sizeof(uint32_t));
	if ((uintptr_t)state > 0) {
		*state = TIMERSPY_CREATED;
		*(uintptr_t *)out_handle = (uintptr_t)state;
	}
	return ESP_OK;
}

esp_err_t esp_timer_delete(esp_timer_handle_t timer)
{
	free(timer);
	return ESP_OK;
}

esp_err_t esp_timer_start_periodic(esp_timer_handle_t timer, uint64_t period_us)
{
	uint32_t *state = (uint32_t *)timer;
	*state = TIMERSPY_STARTED_PERIODIC;
	return ESP_OK;
}

esp_err_t esp_timer_start_once(esp_timer_handle_t timer, uint64_t timeout_us)
{
	uint32_t *state = (uint32_t *)timer;
	*state = TIMERSPY_STARTED;
	return ESP_OK;
}

esp_err_t esp_timer_stop(esp_timer_handle_t timer)
{
	uint32_t *state = (uint32_t *)timer;
	*state = TIMERSPY_STOPPED;
	return ESP_OK;
}

uint32_t apptimerspy_get_state(apptimer_t *timer)
{
	esp_timer_handle_t handle = (esp_timer_handle_t) *(uintptr_t *)timer;
	uint32_t *state = (uint32_t *)handle;
	return *state;
}
