/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_ACTOR_TIMER_H
#define LIBMCU_ACTOR_TIMER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/actor.h"

struct actor_timer;

int actor_timer_init(void *timerpool, size_t timerpool_size);
struct actor_timer *actor_timer_new(struct actor *actor, struct actor_msg *msg,
		uint32_t millisec_delay);
int actor_timer_cancel(struct actor_timer *timer);
int actor_timer_step(uint32_t elapsed_ms);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ACTOR_TIMER_H */
