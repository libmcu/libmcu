/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ratelim.h"
#include "libmcu/compiler.h"

#include <zephyr/kernel.h>

LIBMCU_WEAK ratelim_time_t ratelim_get_time_seconds(void)
{
	const int64_t uptime_ms = k_uptime_get();

	if (uptime_ms < 0) {
		return 0;
	}

	return (ratelim_time_t)((uint64_t)uptime_ms / 1000U);
}
