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

typedef uint16_t metric_key_t;

#define metrics_set(k, v)
#define metrics_get(k)
#define metrics_increase(k)
#define metrics_increase_by(k, n)
#define metrics_reset()
#define metrics_iterate(f, a)
#define metrics_collect(a, b)
#define metrics_count()
#define metrics_init(x)
#define metrics_is_set(x)
#if !defined(METRICS_NO_KEY_STRING)
#define metrics_stringify_key(k)
#endif

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_METRICS_H */
