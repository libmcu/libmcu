/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/logging_overrides.h"
#include "libmcu/compiler.h"

LIBMCU_WEAK void logging_lock_init(void)
{
	/* Platform specific implementation */
}

LIBMCU_WEAK void logging_lock(void)
{
	/* Platform specific implementation */
}

LIBMCU_WEAK void logging_unlock(void)
{
	/* Platform specific implementation */
}
