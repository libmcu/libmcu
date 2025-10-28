/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
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

int actor_timer_init(void *mem, size_t memsize);

struct actor_timer *actor_timer_new(struct actor *actor,
		struct actor_msg *msg, uint32_t millisec_delay);
int actor_timer_delete(struct actor_timer *timer);

int actor_timer_start(struct actor_timer *timer);
int actor_timer_stop(struct actor_timer *timer);

int actor_timer_step(uint32_t elapsed_ms);

size_t actor_timer_cap(void);
size_t actor_timer_len(void);

size_t actor_timer_count_messages(struct actor *actor);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ACTOR_TIMER_H */
