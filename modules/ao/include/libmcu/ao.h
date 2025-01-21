/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_AO_H
#define LIBMCU_AO_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include <semaphore.h>

#if !defined(AO_EVENT_MAXLEN)
/** The maximum event queue length. It should be power of 2 or some of space
 * will be wasted. */
#define AO_EVENT_MAXLEN			16U
#endif

struct ao;
struct ao_event;

typedef void (*ao_dispatcher_t)(struct ao * const ao,
		const struct ao_event * const event);


struct ao_event_queue {
	const struct ao_event *events[AO_EVENT_MAXLEN];
	uint16_t index;
	uint16_t outdex;
};

struct ao {
	ao_dispatcher_t dispatch;
	struct ao_event_queue queue;

	sem_t event;

	pthread_t thread;
	pthread_attr_t attr;
};

struct ao *ao_create(struct ao * const ao,
		size_t stack_size_bytes, int priority);
struct ao *ao_create_static(size_t stack_size_bytes, int priority);
void ao_destroy(struct ao * const ao);

int ao_start(struct ao * const ao, ao_dispatcher_t dispatcher);
int ao_stop(struct ao * const ao);

/**
 * @brief Post an event to dispatch
 *
 * @param ao instance
 * @param event to dispatch
 *
 * @return 0 on success otherwise a negative error code
 *
 * @attention The event which can be any kind of data defined by user must be
 * kept until consumed by the dispatcher since only the reference to it copied
 * internally. This function does not hard-copy the event.
 */
int ao_post(struct ao * const ao, const struct ao_event * const event);
int ao_post_defer(struct ao * const ao, const struct ao_event * const event,
		uint32_t millisec_delay);
int ao_post_repeat(struct ao * const ao, const struct ao_event * const event,
		uint32_t millisec_delay, uint32_t millisec_interval);

/**
 * @brief Post an event if the same event not already in the queue and not
 *        armed yet
 *
 * @param ao instance
 * @param event to dispatch
 *
 * @return 0 on success otherwise a negative error code
 */
int ao_post_if_unique(struct ao * const ao,
		const struct ao_event * const event);
int ao_post_defer_if_unique(struct ao * const ao,
		const struct ao_event * const event, uint32_t millisec_delay);
int ao_post_repeat_if_unique(struct ao * const ao,
		const struct ao_event * const event,
		uint32_t millisec_delay, uint32_t millisec_interval);

/**
 * @brief Cancel out deferred events in the queue
 *
 * @param ao instance
 * @param event to be canceled out
 *
 * @return number of events canceled out if >= 0. or a negative error code
 */
int ao_cancel(const struct ao * const ao, const struct ao_event * const event);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_AO_H */
