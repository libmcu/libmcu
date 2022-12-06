/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ao_timer.h"

#include <stdbool.h>
#include <errno.h>
#include <string.h>

#if !defined(AO_DEBUG)
#define AO_DEBUG(...)
#endif
#if !defined(AO_WARN)
#define AO_WARN(...)
#endif

struct ao_timer {
	struct ao *ao;
	const struct ao_event *event;
	uint32_t timeout_ms;
	uint32_t interval_ms;
};

static volatile bool initialized;
static pthread_mutex_t pool_lock;
static struct ao_timer timer_pool[AO_TIMER_MAXLEN];

static void initialize(void)
{
	AO_DEBUG("initializing ao_timer\n");
	pthread_mutex_init(&pool_lock, NULL);
	memset(timer_pool, 0, sizeof(timer_pool));
	initialized = true;
}

static bool is_allocated(const struct ao_timer * const timer)
{
	return timer->ao != NULL;
}

static struct ao_timer *alloc_timer(void)
{
	for (unsigned int i = 0; i < AO_TIMER_MAXLEN; i++) {
		if (!is_allocated(&timer_pool[i])) {
			AO_DEBUG("%p allocated\n", &timer_pool[i]);
			return &timer_pool[i];
		}
	}

	return NULL;
}

static void free_timer(struct ao_timer * const timer)
{
	memset(timer, 0, sizeof(*timer));
	AO_DEBUG("%p free\n", timer);
}

static int free_timers_by_event(const struct ao_event * const event,
		const struct ao * const ao)
{
	int count = 0;

	for (unsigned int i = 0; i < AO_TIMER_MAXLEN; i++) {
		struct ao_timer *timer = &timer_pool[i];
		if (is_allocated(timer) &&
				timer->event == event &&
				timer->ao == ao) {
			free_timer(timer);
			count++;
		}
	}

	return count;
}

static int add_timer(struct ao * const ao, const struct ao_event * const event,
		uint32_t timeout_ms, uint32_t interval_ms)
{
	struct ao_timer *timer = alloc_timer();

	if (!timer) {
		AO_WARN("allocation failure for a timer\n");
		return -ENOSPC;
	}

	timer->ao = ao;
	timer->event = event;
	timer->timeout_ms = timeout_ms;
	timer->interval_ms = interval_ms;

	AO_DEBUG("%p armed\n", timer);

	return 0;
}

static void update_timeout(struct ao_timer * const timer, uint32_t elapsed_ms)
{
	if (elapsed_ms >= timer->timeout_ms) {
		timer->timeout_ms = 0;
		return;
	}

	timer->timeout_ms -= elapsed_ms;
}

static bool is_timed_out(const struct ao_timer * const timer)
{
	return timer->timeout_ms == 0;
}

static void process_timer(struct ao_timer * const timer, uint32_t elapsed_ms)
{
	if (!is_allocated(timer)) {
		return;
	}

	update_timeout(timer, elapsed_ms);
	if (!is_timed_out(timer)) {
		return;
	}

	/* will try again in the next step in case of failure */
	if (ao_post(timer->ao, timer->event) != 0) {
		return;
	}

	if (!timer->interval_ms) {
		free_timer(timer);
		AO_DEBUG("%p disarmed\n", timer);
	}

	timer->timeout_ms = timer->interval_ms;
}

static void do_step(uint32_t elapsed_ms)
{
	for (unsigned int i = 0; i < AO_TIMER_MAXLEN; i++) {
		struct ao_timer *timer = &timer_pool[i];
		process_timer(timer, elapsed_ms);
	}
}

int ao_timer_add(struct ao * const ao, const struct ao_event * const event,
		uint32_t timeout_ms, uint32_t interval_ms)
{
	if (!initialized) {
		initialize();
	}

	int rc;

	pthread_mutex_lock(&pool_lock);
	rc = add_timer(ao, event, timeout_ms, interval_ms);
	pthread_mutex_unlock(&pool_lock);

	return rc;
}

int ao_timer_cancel(const struct ao * const ao,
		const struct ao_event * const event)
{
	int rc;

	pthread_mutex_lock(&pool_lock);
	rc = free_timers_by_event(event, ao);
	pthread_mutex_unlock(&pool_lock);

	return rc;
}

void ao_timer_step(uint32_t elapsed_ms)
{
	pthread_mutex_lock(&pool_lock);
	do_step(elapsed_ms);
	pthread_mutex_unlock(&pool_lock);
}

void ao_timer_reset(void)
{
	initialize();
}
