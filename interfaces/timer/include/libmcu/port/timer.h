/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_TIMER_PORT_H
#define LIBMCU_TIMER_PORT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/timer.h"

struct timer *timer_port_create(bool periodic,
		timer_callback_t callback, void *arg);
int timer_port_delete(struct timer *timer);
int timer_port_enable(struct timer *timer);
int timer_port_disable(struct timer *timer);
int timer_port_start(struct timer *timer, uint32_t timeout_ms);
int timer_port_restart(struct timer *timer, uint32_t timeout_ms);
int timer_port_stop(struct timer *timer);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_TIMER_PORT_H */
