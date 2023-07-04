/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_TIMER_H
#define LIBMCU_TIMER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

struct timer {
	bool periodic;
	bool enabled;
};

typedef void (*timer_callback_t)(struct timer *timer, void *ctx);

struct timer *timer_create(bool periodic, timer_callback_t callback, void *arg);
int timer_delete(struct timer *timer);
int timer_enable(struct timer *timer);
int timer_disable(struct timer *timer);
int timer_start(struct timer *timer, uint32_t timeout_ms);
int timer_restart(struct timer *timer, uint32_t timeout_ms);
int timer_stop(struct timer *timer);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_TIMER_H */
