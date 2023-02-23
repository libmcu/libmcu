/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/timext.h"
#include <zephyr/kernel.h>

#define time_after(goal, chasing)	((long)(goal)    - (long)(chasing) < 0)

void timeout_set(unsigned long *goal, unsigned long msec)
{
	*goal = k_uptime_get_32() + msec;
}

bool timeout_is_expired(unsigned long goal)
{
	return time_after(goal, k_uptime_get_32());
}

void sleep_ms(unsigned long msec)
{
	k_sleep(K_MSEC(msec));
}
