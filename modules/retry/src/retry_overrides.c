/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/retry_overrides.h"
#include "libmcu/compiler.h"

int LIBMCU_WEAK retry_generate_random(void)
{
	return 0;
}

void LIBMCU_WEAK retry_sleep_ms(unsigned int msec)
{
	unused(msec);
}
