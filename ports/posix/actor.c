/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/actor_overrides.h"
#include "libmcu/actor_timer.h"

#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#if !defined(ACTOR_TIMER_INTERVAL_MS)
#define ACTOR_TIMER_INTERVAL_MS		50UL
#endif

static pthread_mutex_t actor_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t timer_thread;
static bool timer_thread_running = false;

static void loop(struct timespec *curr, struct timespec *prev,
		struct timespec *interval)
{
	nanosleep(interval, NULL);

	clock_gettime(CLOCK_MONOTONIC, curr);

	int64_t delta_sec = (int64_t)(curr->tv_sec - prev->tv_sec);
	int64_t delta_nsec = (int64_t)(curr->tv_nsec - prev->tv_nsec);
	uint64_t elapsed_ns = (uint64_t)(delta_sec * 1000000000LL + delta_nsec);
	uint32_t elapsed_ms = (uint32_t)(elapsed_ns / 1000000ULL);

	if (elapsed_ms > 0) {
		actor_timer_step(elapsed_ms);
	}

	*prev = *curr;
}

static void *timer_loop(void *arg)
{
	(void)arg;

	struct timespec interval;
	interval.tv_sec = ACTOR_TIMER_INTERVAL_MS / 1000;
	interval.tv_nsec = (ACTOR_TIMER_INTERVAL_MS % 1000) * 1000000L;

	struct timespec prev_time, curr_time;
	clock_gettime(CLOCK_MONOTONIC, &prev_time);

	while (timer_thread_running) {
		loop(&curr_time, &prev_time, &interval);
	}

	return NULL;
}

void actor_timer_boot(void)
{
	timer_thread_running = true;
	pthread_create(&timer_thread, NULL, timer_loop, NULL);
	pthread_detach(timer_thread);
}

void actor_lock(void)
{
	pthread_mutex_lock(&actor_mutex);
}

void actor_unlock(void)
{
	pthread_mutex_unlock(&actor_mutex);
}
