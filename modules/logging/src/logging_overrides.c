/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/logging_overrides.h"
#include "libmcu/compiler.h"

LIBMCU_WEAK void logging_lock_init(void)
{
}

LIBMCU_WEAK void logging_lock(void)
{
}

LIBMCU_WEAK void logging_unlock(void)
{
}
