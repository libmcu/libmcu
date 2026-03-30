/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_METRICS_SCHEMA_H
#define LIBMCU_METRICS_SCHEMA_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "libmcu/compiler.h"

typedef enum {
	METRIC_CLASS_UNTYPED    = 0,
	METRIC_CLASS_COUNTER    = 1,
	METRIC_CLASS_GAUGE      = 2,
	METRIC_CLASS_PERCENTAGE = 3,
	METRIC_CLASS_TIMER      = 4,
	METRIC_CLASS_BYTES      = 5,
} metric_class_t;

typedef enum {
	METRIC_UNIT_NONE = 0,
	METRIC_UNIT_MS   = 1,
	METRIC_UNIT_US   = 2,
	METRIC_UNIT_S    = 3,
	METRIC_UNIT_ms   = METRIC_UNIT_MS,
	METRIC_UNIT_us   = METRIC_UNIT_US,
	METRIC_UNIT_s    = METRIC_UNIT_S,
} metric_unit_t;

struct metric_schema {
	metric_class_t type;
	metric_unit_t  unit;
	int32_t        range_min;
	int32_t        range_max;
} LIBMCU_PACKED;

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_METRICS_SCHEMA_H */
