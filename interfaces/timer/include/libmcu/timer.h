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

struct timer;

typedef void (*timer_callback_t)(struct timer *self, void *ctx);

struct timer_api {
	int (*enable)(struct timer *self);
	int (*disable)(struct timer *self);
	int (*start)(struct timer *self, uint32_t timeout_ms);
	int (*restart)(struct timer *self, uint32_t timeout_ms);
	int (*stop)(struct timer *self);
};

static inline int timer_enable(struct timer *self) {
	return ((struct timer_api *)self)->enable(self);
}

static inline int timer_disable(struct timer *self) {
	return ((struct timer_api *)self)->disable(self);
}

static inline int timer_start(struct timer *self, uint32_t timeout_ms) {
	return ((struct timer_api *)self)->start(self, timeout_ms);
}

static inline int timer_restart(struct timer *self, uint32_t timeout_ms) {
	return ((struct timer_api *)self)->restart(self, timeout_ms);
}

static inline int timer_stop(struct timer *self) {
	return ((struct timer_api *)self)->stop(self);
}

struct timer *timer_create(bool periodic, timer_callback_t callback, void *arg);
int timer_delete(struct timer *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_TIMER_H */
