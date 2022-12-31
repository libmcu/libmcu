/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/metrics_overrides.h"
#include "libmcu/compiler.h"
#include <string.h>

LIBMCU_WEAK void metrics_lock(void)
{
}

LIBMCU_WEAK void metrics_unlock(void)
{
}

LIBMCU_WEAK size_t metrics_encode_header(void *buf, size_t bufsize,
		uint32_t nr_total, uint32_t nr_updated)
{
	unused(buf);
	unused(bufsize);
	unused(nr_total);
	unused(nr_updated);
	return 0;
}

LIBMCU_WEAK size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value)
{
	unused(key);

	if (bufsize < sizeof(value)) {
		return 0;
	}

	memcpy(buf, &value, sizeof(value));

	return sizeof(value);
}
