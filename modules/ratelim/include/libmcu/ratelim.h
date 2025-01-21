/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_RATELIM_H
#define LIBMCU_RATELIM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>

typedef enum {
	RATELIM_UNIT_SECOND,
	RATELIM_UNIT_MINUTE,
	RATELIM_UNIT_HOUR,
} ratelim_unit_t;

struct ratelim {
	uint32_t capacity; /**< maximum requests the bucket can hold */
	uint32_t current_load; /**< current number of requests in the bucket */
	uint32_t leak_rate; /**< rate at which the bucket leaks tokens */
	uint32_t leak_time_buffer; /**< time buffer for leak rate calculation */
	time_t last_update; /**< last time the bucket was updated */
};

typedef void (*ratelim_format_func_t)(const char *, va_list);

/**
 * @brief Initialize a rate limiter bucket.
 *
 * This function initializes a rate limiter bucket with a specified capacity,
 * leak rate, and unit of measurement. The unit parameter allows for flexible
 * configuration of the rate limiter based on different time units or other
 * metrics.
 *
 * @param[in] bucket Pointer to the rate limiter bucket to initialize.
 * @param[in] unit Unit of measurement for the rate limiter (e.g., time unit).
 * @param[in] cap The capacity of the rate limiter bucket.
 * @param[in] leak_rate The rate at which the bucket leaks tokens.
 *
 * @note The maximum leak rate when using the second unit is
 *       4294967295/3600=1193046 tokens per hour, when using the minute unit is
 *       4294967295/60=71582788 tokens per hour, and when using the hour unit is
 *       4294967295 tokens per hour.
 */
void ratelim_init(struct ratelim *bucket, const ratelim_unit_t unit,
		const uint32_t cap, const uint32_t leak_rate);

/**
 * @brief Request a token from the rate limiter bucket.
 *
 * This function requests a token from the rate limiter bucket. If a token is
 * available, it is consumed and the function returns true. Otherwise, it
 * returns false.
 *
 * @param[in] bucket Pointer to the rate limiter bucket.
 *
 * @return bool True if a token was successfully requested, false otherwise.
 */
bool ratelim_request(struct ratelim *bucket);

/**
 * @brief Request multiple tokens from the rate limiter bucket.
 *
 * This function requests a specified number of tokens from the rate limiter
 * bucket. If the requested number of tokens are available, they are consumed
 * and the function returns true. Otherwise, it returns false.
 *
 * @param[in] bucket Pointer to the rate limiter bucket.
 * @param[in] n The number of tokens to request.
 *
 * @return bool True if the requested number of tokens were successfully
 *         requested, false otherwise.
 */
bool ratelim_request_ext(struct ratelim *bucket, const uint32_t n);

/**
 * @brief Request a token and call a formatting function if successful.
 *
 * This function requests a token from the rate limiter bucket. If a token is
 * available, it is consumed and the specified formatting function is called
 * with the provided arguments. The function returns true if the token was
 * successfully requested and the function was called, false otherwise.
 *
 * @param[in] bucket Pointer to the rate limiter bucket.
 * @param[in] func Formatting function to call if the token request is
 *            successful.
 * @param[in] ... Additional arguments to pass to the formatting function.
 *
 * @return bool True if the token was successfully requested and the formatting
 *         function was called, false otherwise.
 */
bool ratelim_request_format(struct ratelim *bucket,
		ratelim_format_func_t func, const char *format, ...);

/**
 * @brief Check if the rate limiter bucket is full.
 *
 * This function checks if the rate limiter bucket is full, meaning no more
 * tokens can be requested until some tokens are leaked.
 *
 * @param[in] bucket Pointer to the rate limiter bucket.
 *
 * @return bool True if the bucket is full, false otherwise.
 */
bool ratelim_full(struct ratelim *bucket);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RATELIM_H */
