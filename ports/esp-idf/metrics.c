/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/metrics.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

void metrics_lock(void)
{
	taskENTER_CRITICAL(&spinlock);
}

void metrics_unlock(void)
{
	taskEXIT_CRITICAL(&spinlock);
}
