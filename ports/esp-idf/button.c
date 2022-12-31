/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/button_overrides.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static portMUX_TYPE btn_spinlock = portMUX_INITIALIZER_UNLOCKED;

void button_lock(void)
{
	taskENTER_CRITICAL(&btn_spinlock);
}

void button_unlock(void)
{
	taskEXIT_CRITICAL(&btn_spinlock);
}
