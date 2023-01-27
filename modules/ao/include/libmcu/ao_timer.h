/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_AO_TIMER_H
#define LIBMCU_AO_TIMER_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/ao.h"
#include <stdbool.h>

#if !defined(AO_TIMER_MAXLEN)
#define AO_TIMER_MAXLEN				8U
#endif
#if !defined(AO_TIMER_SCAN_INTERVAL_MS)
#define AO_TIMER_SCAN_INTERVAL_MS		50U
#endif

int ao_timer_add(struct ao * const ao, const struct ao_event * const event,
		uint32_t timeout_ms, uint32_t interval_ms);
int ao_timer_cancel(const struct ao * const ao,
		const struct ao_event * const event);
bool ao_timer_is_armed(const struct ao * const ao,
		const struct ao_event * const event);
void ao_timer_step(uint32_t elapsed_ms);
void ao_timer_reset(void);

int ao_timer_init(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_AO_TIMER_H */
