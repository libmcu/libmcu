/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ao_overrides.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static portMUX_TYPE ao_spinlock = portMUX_INITIALIZER_UNLOCKED;

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
