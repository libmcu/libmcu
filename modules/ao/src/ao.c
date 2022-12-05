/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ao.h"

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdbool.h>

#include "libmcu/compiler.h"
#include "libmcu/bitops.h"

LIBMCU_ASSERT(AO_EVENT_MAXLEN < UINT16_MAX);

#if !defined(AO_DEBUG)
#define AO_DEBUG(...)
#endif
#if !defined(AO_WARN)
#define AO_WARN(...)
#endif
#if !defined(AO_ERROR)
#define AO_ERROR(...)
#endif

struct ao {
	ao_dispatcher_t dispatch;

	sem_t event;

	pthread_mutex_t lock;
	struct ao_event_queue {
		const struct ao_event *events[AO_EVENT_MAXLEN];
		uint16_t index;
		uint16_t outdex;
	} queue;

	pthread_t thread;
	pthread_attr_t attr;
	int priority;
};

static uint16_t get_index(uint16_t index)
{
	const uint16_t cap = (uint16_t)(1U << (flsl(AO_EVENT_MAXLEN) - 1));
	return index & (cap - 1);
}

static void increse_index(uint16_t * const index)
{
	*index = *index + 1;
}

static uint16_t get_queue_len(const struct ao_event_queue * const q)
{
	return q->index - q->outdex;
}

static bool is_queue_empty(const struct ao_event_queue * const q)
{
	return get_queue_len(q) == 0;
}

static bool is_queue_full(const struct ao_event_queue * const q)
{
	return get_queue_len(q) >= AO_EVENT_MAXLEN;
}

static const struct ao_event *pop_event(struct ao_event_queue * const q)
{
	if (is_queue_empty(q)) {
		return NULL;
	}

	const struct ao_event *event = q->events[get_index(q->outdex)];
	increse_index(&q->outdex);

	return event;
}

static bool push_event(struct ao_event_queue * const q,
		const struct ao_event *event)
{
	if (is_queue_full(q)) {
		return false;
	}

	const struct ao_event **entry = &q->events[get_index(q->index)];
	increse_index(&q->index);
	*entry = event;

	return true;
}

static void *ao_task(void *e)
{
	AO_DEBUG("%p task started", e);

	struct ao * const ao = (struct ao * const)e;
	ao_post(ao, 0);

	while (1) {
		sem_wait(&ao->event);

		const struct ao_event * const event = pop_event(&ao->queue);

		AO_DEBUG("%p dispatch event: %p", ao, event);
		(*ao->dispatch)(ao, event);
	}

	AO_DEBUG("%p task termination", ao);
	pthread_exit(NULL);
	return NULL;
}

int ao_post(struct ao * const ao, const struct ao_event * const event)
{
	AO_DEBUG("%p received event: %p", ao, event);

	pthread_mutex_lock(&ao->lock);
	bool ok = push_event(&ao->queue, event);
	pthread_mutex_unlock(&ao->lock);

	if (ok) {
		sem_post(&ao->event);
	} else {
		AO_WARN("%p queue full", ao);
	}

	return ok? 0 : -ENOSPC;
}

int ao_start(struct ao * const ao, ao_dispatcher_t dispatcher)
{
	ao->dispatch = dispatcher;

	if (pthread_create(&ao->thread, &ao->attr, ao_task, ao) != 0) {
		AO_ERROR("cannot create new thread");
		return -EFAULT;
	}

	return 0;
}

int ao_stop(struct ao * const ao)
{
	/* FIXME: make sure no events in the queue to be processed */
	AO_DEBUG("%p task termination", ao);
	pthread_cancel(ao->thread);
	return 0;
}

void ao_destroy(struct ao * const ao)
{
	memset(ao, 0, sizeof(*ao));
}

struct ao *ao_create(size_t stack_size_bytes, int priority)
{
	static struct ao ao = { 0, };

	pthread_attr_init(&ao.attr);
	pthread_attr_setstacksize(&ao.attr, stack_size_bytes);
	pthread_attr_setdetachstate(&ao.attr, PTHREAD_CREATE_DETACHED);

	/* TODO: apply the requested task priority to the task */
	ao.priority = priority;

	pthread_mutex_init(&ao.lock, NULL);

	if (sem_init(&ao.event, 0, 0) != 0) {
		return NULL;
	}

	return &ao;
}
