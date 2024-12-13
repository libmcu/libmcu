/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_METRICS_H
#define LIBMCU_METRICS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if !defined(METRICS_USER_DEFINES)
#define METRICS_USER_DEFINES		"metrics.def"
#endif

#define METRICS_VALUE(x)		((metric_value_t)(x))

enum {
#define METRICS_DEFINE(key)		key,
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
};

typedef uint16_t metric_key_t;
typedef int32_t metric_value_t;

/**
 * @brief Sets the value of a specific metric.
 *
 * This function sets the value of the specified metric key to the given
 * integer value.
 *
 * @param[in] key The metric key to be set.
 * @param[in] val The value to set for the specified metric key.
 */
void metrics_set(const metric_key_t key, const metric_value_t val);

/**
 * @brief Set the metric value if it is less than the current value.
 *
 * This function sets the metric value for the given key if the provided
 * value is less than the current value of the metric.
 *
 * @param[in] key The key identifying the metric.
 * @param[in] val The value to set if it is less than the current value.
 */
void metrics_set_if_min(const metric_key_t key, const metric_value_t val);

/**
 * @brief Set the metric value if it is greater than the current value.
 *
 * This function sets the metric value for the given key if the provided
 * value is greater than the current value of the metric.
 *
 * @param[in] key The key identifying the metric.
 * @param[in] val The value to set if it is greater than the current value.
 */
void metrics_set_if_max(const metric_key_t key, const metric_value_t val);

/**
 * @brief Set the maximum and minimum metric values.
 *
 * This function sets the maximum and minimum metric values for the given
 * keys based on the provided value. If the provided value is greater than
 * the current maximum value, it updates the maximum metric. If the provided
 * value is less than the current minimum value, it updates the minimum metric.
 *
 * @param[in] k_max The key identifying the maximum metric.
 * @param[in] k_min The key identifying the minimum metric.
 * @param[in] val The value to compare and set as the new maximum or minimum.
 */
void metrics_set_max_min(const metric_key_t k_max, const metric_key_t k_min,
		const metric_value_t val);

/**
 * @brief Retrieves the value of a specific metric.
 *
 * This function retrieves the current value of the specified metric key.
 *
 * @param[in] key The metric key to retrieve the value for.
 *
 * @return metric_value_t The current value of the specified metric key.
 */
metric_value_t metrics_get(const metric_key_t key);

/**
 * @brief Increases the value of a specific metric by 1.
 *
 * This function increments the value of the specified metric key by 1.
 *
 * @param[in] key The metric key to be increased.
 */
void metrics_increase(const metric_key_t key);

/**
 * @brief Increases the value of a specific metric by a given amount.
 *
 * This function increments the value of the specified metric key by the
 * given integer amount.
 *
 * @param[in] key The metric key to be increased.
 * @param[in] n The amount to increase the value of the specified metric key by.
 */
void metrics_increase_by(const metric_key_t key, const metric_value_t n);

/**
 * @brief Resets all metrics to their default values.
 *
 * This function resets all metrics to their default values, typically zero.
 */
void metrics_reset(void);

/**
 * @brief Checks if a specific metric is set.
 *
 * This function checks if the specified metric key has been set to a
 * non-default value.
 *
 * @param[in] key The metric key to check.
 *
 * @return bool True if the metric key is set, false otherwise.
 */
bool metrics_is_set(const metric_key_t key);

/**
 * @brief Traversing all metrics firing callback
 *
 * @param callback_each callback to be fired every metric
 * @param ctx context to be used
 *
 * @warn This function does not guarantee synchronization. Any metrics may be
 *       updated while the callback is running.
 */
void metrics_iterate(void (*callback_each)(const metric_key_t key,
				const metric_value_t value, void *ctx),
		void *ctx);

/**
 * @brief Collects all metrics into the provided buffer.
 *
 * This function collects all current metrics and stores them in the
 * provided buffer. The buffer should be large enough to hold all the
 * metrics data.
 *
 * @param[out] buf Pointer to the buffer where metrics data will be stored.
 * @param[in] bufsize Size of the buffer in bytes.
 *
 * @return size_t The number of bytes written to the buffer.
 */
size_t metrics_collect(void *buf, const size_t bufsize);

/**
 * @brief Retrieves the count of all metrics.
 *
 * This function returns the total number of metrics currently being tracked.
 *
 * @return size_t The count of all metrics.
 */
size_t metrics_count(void);

/**
 * @brief Initializes the metrics module.
 *
 * This function initializes the metrics module, setting up any necessary
 * data structures and state. If the force parameter is true, the
 * initialization will be forced even if the module is already initialized.
 *
 * @param force If true, forces re-initialization of the metrics module.
 */
void metrics_init(const bool force);

#if !defined(METRICS_NO_KEY_STRING)
/**
 * @brief Converts a metric key to its string representation.
 *
 * This function converts the specified metric key to its corresponding
 * string representation. This can be useful for logging or displaying
 * metric keys in a human-readable format.
 *
 * @param[in] key The metric key to be converted to a string.
 *
 * @return const char* The string representation of the specified metric key.
 */
const char *metrics_stringify_key(const metric_key_t key);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_METRICS_H */
