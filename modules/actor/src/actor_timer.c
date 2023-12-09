/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/actor_timer.h"
#include "libmcu/actor_overrides.h"

#include <errno.h>

#include "libmcu/assert.h"
#include "libmcu/compiler.h"

#if !defined(ACTOR_DEBUG)
#define ACTOR_DEBUG(...)
#endif
#if !defined(ACTOR_INFO)
#define ACTOR_INFO(...)
#endif
#if !defined(ACTOR_WARN)
#define ACTOR_WARN(...)
#endif

struct actor_timer {
	struct list link;

	struct actor *actor;
	struct actor_msg *msg;

	uint32_t timeout_ms;
};

static struct {
	struct list timer_free;
	struct list timer_armed;
	size_t cap;
} m;

static int add_to_list(struct list *entry, struct list *head)
{
	struct list *p;

	list_for_each(p, head) {
		if (p == entry) {
			ACTOR_WARN("the timer(%p) exists in the queue(%p)",
					entry, head);
			return -EALREADY;
		}
	}

	list_add_tail(entry, head);

	return 0;
}

static struct actor_timer *alloc_timer(struct list *head)
{
	struct list *p = list_first(head);

	if (p == head) {
		return NULL;
	}

	list_del(p, head);

	struct actor_timer *timer = list_entry(p, struct actor_timer, link);
	ACTOR_INFO("timer allocated: %p", timer);

	return timer;
}

static void free_timer(struct actor_timer *timer, struct list *head)
{
	add_to_list(&timer->link, head);
	ACTOR_INFO("timer free: %p", timer);
}

static void update_timeout(struct actor_timer *timer, uint32_t elapsed_ms)
{
	if (elapsed_ms >= timer->timeout_ms) {
		timer->timeout_ms = 0;
		return;
	}

	timer->timeout_ms -= elapsed_ms;
}

static bool is_timed_out(const struct actor_timer *timer)
{
	return timer->timeout_ms == 0;
}

size_t actor_timer_cap(void)
{
	return m.cap;
}

size_t actor_timer_len(void)
{
	actor_lock();
	size_t cnt = (size_t)list_count(&m.timer_free);
	actor_unlock();

	return m.cap - cnt;
}

int actor_timer_start(struct actor_timer *timer)
{
	actor_lock();
	add_to_list(&timer->link, &m.timer_armed);
	actor_unlock();

	ACTOR_INFO("timer armed: %p", timer);
	return 0;
}

int actor_timer_stop(struct actor_timer *timer)
{
	struct list *p;

	actor_lock();

	list_for_each(p, &m.timer_armed) {
		if (p == &timer->link) {
			list_del(p, &m.timer_armed);
			break;
		}
	}

	actor_unlock();

	return 0;
}

struct actor_timer *actor_timer_new(struct actor *actor,
		struct actor_msg *msg, uint32_t millisec_delay)
{
	actor_lock();
	struct actor_timer *timer = alloc_timer(&m.timer_free);
	actor_unlock();

	if (timer) {
		timer->actor = actor;
		timer->msg = msg;
		timer->timeout_ms = millisec_delay;
	}

	return timer;
}

int actor_timer_delete(struct actor_timer *timer)
{
	actor_timer_stop(timer);

	actor_lock();
	free_timer(timer, &m.timer_free);
	actor_unlock();

	return 0;
}

int actor_timer_step(uint32_t elapsed_ms)
{
	struct list *p;
	struct list *t;

	actor_lock();

	list_for_each_safe(p, t, &m.timer_armed) {
		struct actor_timer *timer =
			list_entry(p, struct actor_timer, link);
		update_timeout(timer, elapsed_ms);
		if (!is_timed_out(timer)) {
			continue;
		}

		actor_unlock();

		actor_send(timer->actor, timer->msg);

		actor_lock();

		list_del(&timer->link, &m.timer_armed);
		ACTOR_INFO("timer disarmed: %p", timer);
		free_timer(timer, &m.timer_free);
	}

	actor_unlock();

	return 0;
}

int actor_timer_init(void *mem, size_t memsize)
{
	const size_t mask = sizeof(uintptr_t) - 1;
	const size_t remainder = (size_t)mem & mask;

	assert(mem);
	assert(memsize >= (sizeof(struct actor_timer) + remainder));

	const size_t maxbytes = memsize - remainder;
	m.cap = maxbytes / sizeof(struct actor_timer);

	struct actor_timer *timers = (struct actor_timer *)
		(((uintptr_t)mem + mask) & ~mask);

	list_init(&m.timer_free);
	list_init(&m.timer_armed);

	for (size_t i = 0; i < m.cap; i++) {
		add_to_list(&timers[i].link, &m.timer_free);
		ACTOR_DEBUG("free timer entry: %p", &timers[i].link);
	}

	ACTOR_INFO("%lu free timer entries initialized.", m.cap);
	ACTOR_DEBUG("%lu bytes wasted.", memsize - maxbytes);

	return 0;
}
