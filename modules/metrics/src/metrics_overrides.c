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
	/* platform specific implementation */
}

LIBMCU_WEAK void metrics_unlock(void)
{
	/* platform specific implementation */
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
	const uint32_t kval = (const uint32_t)key;
	const size_t len = sizeof(kval) + sizeof(value);

	if (bufsize < len) {
		return 0;
	}

	memcpy(buf, &kval, sizeof(kval));
	memcpy(&buf[sizeof(kval)], &value, sizeof(value));

	return len;
}
