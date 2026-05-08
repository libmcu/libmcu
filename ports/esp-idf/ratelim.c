/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ratelim.h"
#include "libmcu/compiler.h"

#include "esp_timer.h"

LIBMCU_WEAK ratelim_time_t ratelim_get_time_seconds(void)
{
	const int64_t elapsed_usec = esp_timer_get_time();

	if (elapsed_usec < 0) {
		return 0;
	}

	return (ratelim_time_t)((uint64_t)elapsed_usec / 1000000U);
}
