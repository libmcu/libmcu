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
#include <time.h>

void timeout_set(unsigned long *goal, unsigned long msec);
bool timeout_is_expired(unsigned long goal);

void sleep_ms(unsigned long msec);

#if defined(_GNU_SOURCE)
/**
 * @brief Converts an ISO 8601 formatted time string to a time_t value.
 *
 * This function parses an ISO 8601 formatted time string and converts it to a
 * time_t value representing the number of seconds since the Unix epoch
 * (1970-01-01 00:00:00 UTC).
 *
 * @param[in] tstr A pointer to the ISO 8601 formatted time string.
 *
 * @return A time_t value representing the parsed time, or (time_t)-1 if the
 *         parsing fails.
 */
time_t iso8601_convert_to_time(const char *tstr);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_TIMEXT_H */
