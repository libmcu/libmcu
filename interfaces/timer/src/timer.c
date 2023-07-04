/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/timer.h"
#include "libmcu/port/timer.h"

#include <errno.h>

int timer_start(struct timer *timer, uint32_t timeout_ms)
{
	if (!timer->enabled) {
		return -ENODEV;
	}

	return timer_port_start(timer, timeout_ms);
}

int timer_restart(struct timer *timer, uint32_t timeout_ms)
{
	if (!timer->enabled) {
		return -ENODEV;
	}

	return timer_port_restart(timer, timeout_ms);
}

int timer_stop(struct timer *timer)
{
	if (!timer->enabled) {
		return -ENODEV;
	}

	return timer_port_stop(timer);
}

int timer_enable(struct timer *timer)
{
	if (timer->enabled) {
		return -EALREADY;
	}

	int err = timer_port_enable(timer);

	if (!err) {
		timer->enabled = true;
	}

	return err;
}

int timer_disable(struct timer *timer)
{
	if (!timer->enabled) {
		return -ENODEV;
	}

	int err = timer_port_disable(timer);
	timer->enabled = false;

	return err;
}

struct timer *timer_create(bool periodic, timer_callback_t callback, void *arg)
{
	struct timer *timer = timer_port_create(periodic, callback, arg);

	if (timer) {
		timer->periodic = periodic;
	}

	return timer;
}

int timer_delete(struct timer *timer)
{
	timer_disable(timer);
	return timer_port_delete(timer);
}
