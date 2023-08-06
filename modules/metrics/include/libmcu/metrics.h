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

#define METRICS_VALUE(x)		((int32_t)(x))

enum {
#define METRICS_DEFINE(key)		key,
#include METRICS_USER_DEFINES
#undef METRICS_DEFINE
};

typedef uint16_t metric_key_t;

void metrics_set(metric_key_t key, int32_t val);
int32_t metrics_get(metric_key_t key);
void metrics_increase(metric_key_t key);
void metrics_increase_by(metric_key_t key, int32_t n);
void metrics_reset(void);
/**
 * @brief Traversing all metrics firing callback
 *
 * @param callback_each callback to be fired every metric
 * @param ctx context to be used
 *
 * @warn This function does not guarantee synchronization. Any metrics may be
 *       updated while the callback is running.
 */
void metrics_iterate(void (*callback_each)(metric_key_t key, int32_t value,
					   void *ctx), void *ctx);
size_t metrics_collect(void *buf, size_t bufsize);
size_t metrics_count(void);

void metrics_init(bool force);

#if !defined(METRICS_NO_KEY_STRING)
const char *metrics_stringify_key(metric_key_t key);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_METRICS_H */
