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

#include "libmcu/metrics.h"

void metrics_lock_init(void);
void metrics_lock(void);
void metrics_unlock(void);

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
size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_METRICS_OVERRIDES_H */
