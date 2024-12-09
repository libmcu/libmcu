/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_RETRY_H
#define LIBMCU_RETRY_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	RETRY_ERROR_NONE,
	RETRY_ERROR_INVALID_PARAM,
	RETRY_ERROR_NOMEM,
	RETRY_ERROR_EXHAUSTED,
} retry_error_t;

struct retry_param {
	uint16_t max_attempts;
	uint16_t min_backoff_ms;
	uint32_t max_backoff_ms;
	uint16_t max_jitter_ms;
};

struct retry {
	struct retry_param param;

	uint16_t attempts;
	uint32_t previous_backoff_ms;
};

/**
 * @brief Create a new retry instance with the specified parameters.
 *
 * This function initializes a new retry instance using the provided parameters.
 *
 * @param[in] param The parameters for the retry instance.
 *
 * @return A retry_error_t indicating success or failure.
 */
retry_error_t retry_new(const struct retry_param *param);

/**
 * @brief Delete the retry instance.
 *
 * This function deletes the retry instance, freeing any allocated resources.
 *
 * @param[in] self The retry instance to delete.
 */
void retry_delete(struct retry *self);

/**
 * @brief Initialize a static retry instance with the specified parameters.
 *
 * This function initializes a static retry instance using the provided
 * parameters.
 *
 * @param[in] self The static retry instance to initialize.
 * @param[in] param The parameters for the retry instance.
 *
 * @return A retry_error_t indicating success or failure.
 */
retry_error_t retry_new_static(struct retry *self,
		const struct retry_param *param);

/**
 * @brief Reset the retry instance.
 *
 * This function resets the retry instance, clearing any previous state.
 *
 * @param[in] self The retry instance to reset.
 */
void retry_reset(struct retry *self);

/**
 * @brief Check if the retry attempts are exhausted.
 *
 * This function checks if the retry attempts have been exhausted.
 *
 * @param[in] self The retry instance to check.
 *
 * @return true if the retry attempts are exhausted, false otherwise.
 */
bool retry_exhausted(const struct retry *self);

/**
 * @brief Check if the current attempt is the first retry.
 *
 * This function checks if the current attempt is the first retry attempt.
 *
 * @param[in] self The retry instance to check.
 *
 * @return true if the current attempt is the first retry, false otherwise.
 */
bool retry_first(const struct retry *self);

/**
 * @brief Get the current backoff time for the retry instance.
 *
 * This function returns the current backoff time for the retry instance.
 * The backoff time is the amount of time to wait before making the next retry
 * attempt.
 *
 * @param[in] self The retry instance to get the backoff time from.
 *
 * @return The current backoff time in milliseconds.
 */
uint32_t retry_get_backoff(const struct retry *self);

/**
 * @brief Calculate the next backoff time with optional jitter.
 *
 * This function calculates the next backoff time for the retry instance,
 * including an optional jitter value to randomize the backoff time.
 *
 * @param[in]  self The retry instance.
 * @param[out] next_backoff_ms Pointer to store the calculated next backoff
 *             time in milliseconds.
 * @param[in]  jitter_seed The seed value for generating random jitter.
 *
 * @return A retry_error_t indicating success or failure.
 */
retry_error_t retry_backoff(struct retry *self,
		uint32_t *next_backoff_ms, const uint16_t jitter_seed);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RETRY_H */
