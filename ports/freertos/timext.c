/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/timext.h"

#include "FreeRTOS.h"
#include "task.h"

#define time_after(goal, chasing)	((int)(goal)    - (int)(chasing) < 0)

void timeout_set(unsigned int *goal, unsigned int msec)
{
	*goal = xTaskGetTickCount() + pdMS_TO_TICKS(msec);
}

bool timeout_is_expired(unsigned int goal)
{
	return time_after(goal, xTaskGetTickCount());
}

void sleep_ms(unsigned int msec)
{
	vTaskDelay(pdMS_TO_TICKS(msec));
}
