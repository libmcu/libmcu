/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_METRICS_OVERRIDES_H
#define LIBMCU_METRICS_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "libmcu/metrics.h"

void metrics_lock_init(void);
void metrics_lock(void);
void metrics_unlock(void);

/**
 * @brief Returns the device serial number string for encoder metadata.
 *
 * Override this function when an encoder needs device identification in the
 * encoded payload. Returning an empty string is allowed.
 *
 * @return null-terminated serial number string, or an empty string.
 */
const char *metrics_get_serial_number_string(void);

/**
 * @brief Returns the current Unix timestamp for encoder metadata.
 *
 * Override this function to provide a real wall-clock time. Returning 0 is
 * allowed when no RTC is available.
 *
 * @return seconds since Unix epoch (UTC), or 0 if unavailable.
 */
uint64_t metrics_get_unix_timestamp(void);

/**
 * @brief It creates an encoding header.
 *
 * This function is called internally in `metrics_collect()`.
 *
 * @param[in] buf buffer
 * @param[in] bufsize buffer size
 * @param[in] nr_total the number of metrics declared in metrics.def
 * @param[in] nr_updated the number of metrics with non-zero value
 * @return the number of bytes written
 */
size_t metrics_encode_header(void *buf, size_t bufsize,
		uint32_t nr_total, uint32_t nr_updated);

#if defined(METRICS_SCHEMA_IBS)
size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value,
		const struct metric_schema *schema);
#else
size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_METRICS_OVERRIDES_H */
