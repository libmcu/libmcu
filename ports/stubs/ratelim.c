/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ratelim.h"
#include "libmcu/compiler.h"

LIBMCU_WEAK ratelim_time_t ratelim_get_time_seconds(void)
{
	return 0;
}
