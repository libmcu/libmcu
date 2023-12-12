/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_ACTOR_OVERRIDES_H
#define LIBMCU_ACTOR_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/actor.h"

void actor_lock(void);
void actor_unlock(void);

/**
 * @brief Initialize a platform specific timer for actors
 *
 * @note This will be called at the end of @ref actor_timer_init()
 */
void actor_timer_boot(void);

void actor_pre_dispatch_hook(const struct actor *actor,
		const struct actor_msg *msg);
void actor_post_dispatch_hook(const struct actor *actor,
		const struct actor_msg *msg);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ACTOR_OVERRIDES_H */
