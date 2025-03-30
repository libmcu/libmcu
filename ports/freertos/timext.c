/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/timext.h"

#include "FreeRTOS.h"
#include "task.h"

#define time_after(goal, chasing)	((long)(goal) - (long)(chasing) < 0)

void timeout_set(uint32_t *goal, uint32_t msec)
{
	*goal = xTaskGetTickCount() + pdMS_TO_TICKS(msec);
}

bool timeout_is_expired(uint32_t goal)
{
	return time_after(goal, xTaskGetTickCount());
}

void sleep_ms(uint32_t msec)
{
	vTaskDelay(pdMS_TO_TICKS(msec));
}
