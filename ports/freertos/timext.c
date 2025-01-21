/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/timext.h"

#include "FreeRTOS.h"
#include "task.h"

#define time_after(goal, chasing)	((long)(goal) - (long)(chasing) < 0)

void timeout_set(unsigned long *goal, unsigned long msec)
{
	*goal = xTaskGetTickCount() + pdMS_TO_TICKS(msec);
}

bool timeout_is_expired(unsigned long goal)
{
	return time_after(goal, xTaskGetTickCount());
}

void sleep_ms(unsigned long msec)
{
	vTaskDelay(pdMS_TO_TICKS(msec));
}
