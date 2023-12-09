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
	uint32_t interval_ms;
};

static struct list timer_free;
static struct list timer_armed;

static int add_to_list(struct list *entry, struct list *head)
{
	struct list *p;

	list_for_each(p, head) {
		if (p == entry) {
			ACTOR_WARN("the entry(%p) exists in the queue(%p)",
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
	ACTOR_INFO("Allocated: %p", timer);

	return timer;
}

static void free_timer(struct actor_timer *timer, struct list *head)
{
	add_to_list(&timer->link, head);
	ACTOR_INFO("Free: %p", timer);
}

static struct actor_timer *set_timer(struct actor *actor,
		struct actor_msg *msg, uint32_t millisec_delay, bool repeat)
{
	struct actor_timer *timer = alloc_timer(&timer_free);

	if (timer) {
		timer->actor = actor;
		timer->msg = msg;
		timer->timeout_ms = millisec_delay;
		timer->interval_ms = repeat? millisec_delay : 0;

		add_to_list(&timer->link, &timer_armed);
	}

	return timer;
}

static void update_timeout(struct actor_timer *timer, uint32_t elapsed_ms)
{
	if (elapsed_ms >= timer->timeout_ms) {
		timer->timeout_ms = 0;
		return;
	}

	timer->timeout_ms -= elapsed_ms;
}

static bool is_timed_out(struct actor_timer *timer)
{
	return timer->timeout_ms == 0;
}

int actor_timer_delete(struct actor_timer *timer)
{
	struct list *p;

	actor_lock();

	list_for_each(p, &timer_armed) {
		if (p == &timer->link) {
			list_del(p, &timer_armed);
			break;
		}
	}

	free_timer(timer, &timer_free);

	actor_unlock();

	return 0;
}

struct actor_timer *actor_timer_new(struct actor *actor,
		struct actor_msg *msg, uint32_t millisec_delay, bool repeat)
{
	actor_lock();

	struct actor_timer *timer =
		set_timer(actor, msg, millisec_delay, repeat);

	actor_unlock();

	ACTOR_INFO("%p armed", timer);

	return timer;
}

int actor_timer_step(uint32_t elapsed_ms)
{
	struct list *p;
	struct list *t;

	actor_lock();

	list_for_each_safe(p, t, &timer_armed) {
		struct actor_timer *timer =
			list_entry(p, struct actor_timer, link);
		update_timeout(timer, elapsed_ms);
		if (!is_timed_out(timer)) {
			continue;
		}

		actor_unlock();
		actor_send(timer->actor, timer->msg);
		actor_lock();

		if (!timer->interval_ms) {
			list_del(&timer->link, &timer_armed);
			free_timer(timer, &timer_free);
			ACTOR_INFO("%p disarmed", timer);
		}

		timer->timeout_ms = timer->interval_ms;
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
	const size_t maxlen = maxbytes / sizeof(struct actor_timer);

	struct actor_timer *timers = (struct actor_timer *)
		(((uintptr_t)mem + mask) & ~mask);

	list_init(&timer_free);
	list_init(&timer_armed);

	for (size_t i = 0; i < maxlen; i++) {
		add_to_list(&timers[i].link, &timer_free);
		ACTOR_DEBUG("free entry: %p", &timers[i].link);
	}

	ACTOR_INFO("%lu free entries initialized.", maxlen);
	ACTOR_DEBUG("%lu bytes wasted.", memsize - maxbytes);

	return 0;
}
