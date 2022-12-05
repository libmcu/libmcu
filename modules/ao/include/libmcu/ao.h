/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
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

#if !defined(AO_EVENT_MAXLEN)
/** The maximum event queue length. It should be power of 2 or some of space
 * will be wasted. */
#define AO_EVENT_MAXLEN			8U
#endif

struct ao;
struct ao_event;

typedef void (*ao_dispatcher_t)(struct ao * const ao,
		const struct ao_event * const event);

struct ao *ao_create(size_t stack_size_bytes, int priority);
void ao_destroy(struct ao * const ao);

int ao_start(struct ao * const ao, ao_dispatcher_t dispatcher);
int ao_stop(struct ao * const ao);

/**
 * @brief Post an event to dispatch
 *
 * @param ao ao instance
 * @param event to dispatch
 *
 * @return 0 on success otherwise a negative error code
 *
 * @attention The event which can be any kind of data defined by user must be
 * kept until consumed by the dispatcher since only the reference to it copied
 * internally. This function does not hard-copy the event.
 */
int ao_post(struct ao * const ao, const struct ao_event * const event);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_AO_H */
