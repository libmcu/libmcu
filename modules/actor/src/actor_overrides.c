/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/actor_overrides.h"
#include "libmcu/compiler.h"

LIBMCU_WEAK
void actor_lock(void)
{
	/* platform specific implementation */
}

LIBMCU_WEAK
void actor_unlock(void)
{
	/* platform specific implementation */
}

LIBMCU_WEAK
void actor_timer_boot(void)
{
	/* platform specific implementation */
}

LIBMCU_WEAK
void actor_pre_dispatch_hook(const struct actor *actor,
		const struct actor_msg *msg)
{
	/* platform specific implementation */
	unused(actor);
	unused(msg);
}

LIBMCU_WEAK
void actor_post_dispatch_hook(const struct actor *actor,
		const struct actor_msg *msg)
{
	/* platform specific implementation */
	unused(actor);
	unused(msg);
}
