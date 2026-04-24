/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/metrics_overrides.h"
#include "libmcu/compiler.h"
#include <string.h>
#include <errno.h>

LIBMCU_WEAK void metrics_lock(void)
{
	/* platform specific implementation */
}

LIBMCU_WEAK void metrics_unlock(void)
{
	/* platform specific implementation */
}

LIBMCU_WEAK const char *metrics_get_serial_number_string(void)
{
	return "";
}

LIBMCU_WEAK uint64_t metrics_get_unix_timestamp(void)
{
	return 0;
}

LIBMCU_WEAK const char *metrics_get_version_string(void)
{
	return "";
}

#if defined(METRICS_SCHEMA_IBS)
LIBMCU_WEAK size_t metrics_encode_header(void *buf, size_t bufsize,
		uint32_t nr_total, uint32_t nr_updated)
{
	const uint16_t n = (uint16_t)nr_updated;
	const size_t len = sizeof(n);

	unused(nr_total);

	if (buf == NULL) {
		return len;
	}

	if (bufsize < len) {
		return 0;
	}

	memcpy(buf, &n, sizeof(n));
	return len;
}

LIBMCU_WEAK size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value,
		const struct metric_schema *schema)
{
	const uint32_t kval = (uint32_t)key;
	const uint8_t  tval = schema->type;
	const uint8_t  uval = schema->unit;
	const size_t len = sizeof(kval) + sizeof(tval) + sizeof(uval)
			+ sizeof(schema->range_min) + sizeof(schema->range_max)
			+ sizeof(value);

	if (buf == NULL) { /* dry-run: only returns required size */
		return len;
	}

	if (bufsize < len) {
		return 0;
	}

	size_t offset = 0;
	memcpy((uint8_t *)buf + offset, &kval,             sizeof(kval));
	offset += sizeof(kval);
	memcpy((uint8_t *)buf + offset, &tval,             sizeof(tval));
	offset += sizeof(tval);
	memcpy((uint8_t *)buf + offset, &uval,             sizeof(uval));
	offset += sizeof(uval);
	memcpy((uint8_t *)buf + offset, &schema->range_min, sizeof(schema->range_min));
	offset += sizeof(schema->range_min);
	memcpy((uint8_t *)buf + offset, &schema->range_max, sizeof(schema->range_max));
	offset += sizeof(schema->range_max);
	memcpy((uint8_t *)buf + offset, &value,            sizeof(value));

	return len;
}
#else /* METRICS_SCHEMA_IBS */
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

	if (buf == NULL) { /* dry-run: only returns required size */
		return len;
	}

	if (bufsize < len) {
		return 0;
	}

	memcpy(buf, &kval, sizeof(kval));
	memcpy(&((uint8_t *)buf)[sizeof(kval)], &value, sizeof(value));

	return len;
}
#endif /* METRICS_SCHEMA_IBS */
