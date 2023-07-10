/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/logging.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

void logging_lock_init(void)
{
}

void logging_lock(void)
{
	taskENTER_CRITICAL(&spinlock);
}

void logging_unlock(void)
{
	taskEXIT_CRITICAL(&spinlock);
}
