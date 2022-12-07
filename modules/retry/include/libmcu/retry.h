/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_RETRY_H
#define LIBMCU_RETRY_H

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(RETRY_DEBUG)
#define RETRY_DEBUG(...)
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	RETRY_RUNNING		= 0,
	RETRY_EXHAUSTED,
} retry_error_t;

/** Do not touch manually. */
struct retry {
	uint32_t max_backoff_ms;
	uint16_t min_backoff_ms;
	uint16_t max_jitter_ms;
	uint16_t max_attempts;
	uint16_t attempts;
	uint32_t previous_backoff_ms;
};

void retry_init(struct retry *self, uint16_t max_attempts,
		uint32_t max_backoff_ms, uint16_t min_backoff_ms,
		uint16_t max_jitter_ms);

void retry_reset(struct retry *self);

/**
 * @brief Calculate the next backoff time in the unit of milli seconds
 *
 * @param self instance
 *
 * @return the next backoff time in the unit of milli seconds
 */
uint32_t retry_backoff_next(struct retry *self);

/**
 * @brief Calculate the next backoff time in the unit of milli seconds
 *
 * @param self instance
 *
 * @return true if exhausted. false otherwise
 */
bool retry_exhausted(const struct retry *self);

/**
 * @brief Backoff for an amount of time
 *
 * @param self instance
 *
 * @note This is a blocking function sleeping for the backoff time.
 *
 * @return @ref retry_error_t
 */
retry_error_t retry_backoff(struct retry *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RETRY_H */
