/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200809L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE		200809L
#endif

#include "libmcu/ratelim.h"
#include "libmcu/compiler.h"

#include <time.h>

#if !defined(CLOCK_MONOTONIC)
#error "CLOCK_MONOTONIC is required; use LIBMCU_RATELIM_PORT=stubs"
#endif

LIBMCU_WEAK ratelim_time_t ratelim_get_time_seconds(void)
{
	struct timespec now;

	if (clock_gettime(CLOCK_MONOTONIC, &now) != 0 || now.tv_sec < 0) {
		return 0;
	}

	return (ratelim_time_t)now.tv_sec;
}
