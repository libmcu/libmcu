/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/pm.h"
#include "libmcu/board/pm.h"

#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <pthread.h>

static struct pm_item {
	pm_callback_t func;
	void *ctx;
	pm_mode_t mode;
	int8_t priority;
	bool on_exit;
} slots[PM_CALLBACK_MAXLEN];

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static int count_empty_slots(void)
{
	int cnt = 0;

	for (unsigned int i = 0; i < PM_CALLBACK_MAXLEN; i++) {
		struct pm_item *p = slots[i];
		if (!p->func) {
			cnt++;
		}
	}

	return cnt;
}

static void move_right(struct pm_item *p, unsigned int n)
{
	struct pm_item next = p[0];

	while (n--) {
		struct pm_item t = p[1];
		p[1] = next;
		next = t;
		p++;

		if (!next.func) {
			break;
		}
	}
}
//static void move_left

static int register_entry(bool on_exit, pm_mode_t mode, int8_t priority,
		pm_callback_t func, void *arg)
{
	struct pm_item *slot = NULL;

	for (unsigned int i = 0; i < PM_CALLBACK_MAXLEN; i++) {
		struct pm_item *p = slots[i];
		if (!p->func) { /* no empty slot in the middle guaranteed */
			slot = p;
			break;
		} else if (p->mode != mode) {
			continue;
		} else if (p->priority < priority) {
			move_right(p, PM_CALLBACK_MAXLEN - i);
			slot = p;
			break;
		}
	}

	if (!slot) {
		return -EFAULT;
	}

	slot->on_exit = on_exit;
	slot->mode = mode;
	slot->priority = priority;
	slot->func = func;
	slot->ctx = arg;

	return 0;
}

static void dispatch_entries(pm_mode_t mode)
{
	for (unsigned int i = 0; i < PM_CALLBACK_MAXLEN; i++) {
		struct pm_item *p = slots[i];
		if (p->mode != mode || !p->func || p->on_exit) {
			continue;
		}

		(*p->func)(p->ctx);
	}
}

static void dispatch_exits(pm_mode_t mode)
{
	for (unsigned int i = 0; i < PM_CALLBACK_MAXLEN; i++) {
		struct pm_item *p = slots[i];
		if (p->mode != mode || !p->func || !p->on_exit) {
			continue;
		}

		(*p->func)(p->ctx);
	}
}

int pm_enter(pm_mode_t mode)
{
	pthread_mutex_lock(&lock);
	dispatch_entries(mode);

	int rc = pm_board_enter(mode);

	dispatch_exits(mode);
	pthread_mutex_unlock(&lock);

	return rc;
}

int pm_register_entry_callback(pm_mode_t mode, int8_t priority,
		pm_callback_t func, void *arg)
{
	int rc;

	if (!func) {
		return -EINVAL;
	}

	pthread_mutex_lock(&lock);
	if (count_empty_slots() > 0) {
		rc = register_entry(false, mode, priority, func, arg);
	}
	pthread_mutex_unlock(&lock);

	return rc;
}

int pm_register_exit_callback(pm_mode_t mode, int8_t priority,
		pm_callback_t func, void *arg)
{
	int rc;

	if (!func) {
		return -EINVAL;
	}

	pthread_mutex_lock(&lock);
	if (count_empty_slots() > 0) {
		rc = register_entry(true, mode, priority, func, arg);
	}
	pthread_mutex_unlock(&lock);

	return rc;
}

void pm_reset(void)
{
	pthread_mutex_lock(&lock);
	memset(slots, 0, sizeof(slots));
	pthread_mutex_init(&lock, NULL);
}
