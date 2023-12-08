/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/actor_overrides.h"
#include "libmcu/compiler.h"

LIBMCU_WEAK
void actor_lock(void)
{
}

LIBMCU_WEAK
void actor_unlock(void)
{
}
