/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/timext.h"
#include <zephyr/kernel.h>

#define time_after(goal, chasing)	((long)(goal)    - (long)(chasing) < 0)

void timeout_set(uint32_t *goal, uint32_t msec)
{
	*goal = k_uptime_get_32() + msec;
}

bool timeout_is_expired(uint32_t goal)
{
	return time_after(goal, k_uptime_get_32());
}

void sleep_ms(uint32_t msec)
{
	k_sleep(K_MSEC(msec));
}
