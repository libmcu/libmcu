/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ao_overrides.h"
#include "libmcu/ao_timer.h"
#include "libmcu/compiler.h"

LIBMCU_WEAK
void ao_lock(void *ctx)
{
	/* platform specific implementation */
	unused(ctx);
}

LIBMCU_WEAK
void ao_unlock(void *ctx)
{
	/* platform specific implementation */
	unused(ctx);
}

LIBMCU_WEAK
void ao_timer_lock(void)
{
	/* platform specific implementation */
}

LIBMCU_WEAK
void ao_timer_unlock(void)
{
	/* platform specific implementation */
}

LIBMCU_WEAK
void ao_timer_lock_init(void)
{
	/* platform specific implementation */
}

LIBMCU_WEAK
int ao_timer_init(void)
{
	return 0;
}
