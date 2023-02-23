/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_TIMEXT_H
#define LIBMCU_TIMEXT_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>

void timeout_set(unsigned long *goal, unsigned long msec);
bool timeout_is_expired(unsigned long goal);

void sleep_ms(unsigned long msec);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_TIMEXT_H */
