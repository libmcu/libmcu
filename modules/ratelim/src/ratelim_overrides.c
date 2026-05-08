/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ratelim.h"
#include <time.h>
#include "libmcu/compiler.h"

LIBMCU_WEAK ratelim_time_t ratelim_get_time_seconds(void)
{
	const time_t now = time(NULL);

	if (now == (time_t)-1) {
		return 0;
	}

	return (ratelim_time_t)now;
}
