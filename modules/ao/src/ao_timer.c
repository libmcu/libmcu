/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ao_timer.h"
#include "libmcu/ao_overrides.h"

#include <stdbool.h>
#include <errno.h>
#include <string.h>

#if !defined(AO_ASSERT)
#include <assert.h>
#define AO_ASSERT(...)		assert(__VA_ARGS__)
#endif
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
static struct ao_timer timer_pool[AO_TIMER_MAXLEN];

static bool initialize(void)
{
	AO_DEBUG("initializing ao_timer\n");

	ao_timer_lock_init();
	memset(timer_pool, 0, sizeof(timer_pool));

	initialized = true;

	return initialized;
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
		const struct ao * const ao, bool dryrun)
{
	int count = 0;

	for (unsigned int i = 0; i < AO_TIMER_MAXLEN; i++) {
		struct ao_timer *timer = &timer_pool[i];
		if (is_allocated(timer) &&
				timer->event == event &&
				timer->ao == ao) {
			if (!dryrun) {
				free_timer(timer);
			}
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
	if (!initialized && !initialize()) {
		return -EFAULT;
	}

	int rc;

	ao_timer_lock();
	rc = add_timer(ao, event, timeout_ms, interval_ms);
	ao_timer_unlock();

	return rc;
}

int ao_timer_cancel(const struct ao * const ao,
		const struct ao_event * const event)
{
	int rc;

	ao_timer_lock();
	rc = free_timers_by_event(event, ao, false);
	ao_timer_unlock();

	return rc;
}

bool ao_timer_is_armed(const struct ao * const ao,
		const struct ao_event * const event)
{
	int rc;

	ao_timer_lock();
	rc = free_timers_by_event(event, ao, true);
	ao_timer_unlock();

	return rc != 0;
}

void ao_timer_step(uint32_t elapsed_ms)
{
	ao_timer_lock();
	do_step(elapsed_ms);
	ao_timer_unlock();
}

void ao_timer_reset(void)
{
	if (!initialize()) {
		AO_ASSERT(0);
	}
}
