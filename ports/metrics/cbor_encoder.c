/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/metrics.h"

#include "cbor/cbor.h"
#include "cbor/encoder.h"

static cbor_writer_t writer;

size_t metrics_encode_header(void *buf, size_t bufsize,
		uint32_t nr_total, uint32_t nr_updated)
{
	cbor_writer_init(&writer, buf, bufsize);
	cbor_encode_map(&writer, nr_updated);

	return cbor_writer_len(&writer);
}

#if defined(METRICS_SCHEMA_IBS)
size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value,
		const struct metric_schema *schema)
{
	size_t len = cbor_writer_len(&writer);

	cbor_encode_unsigned_integer(&writer, (uint64_t)key);

	/* value → [class, unit, range_min, range_max, value] */
	cbor_encode_array(&writer, 5);

	cbor_encode_unsigned_integer(&writer, (uint64_t)schema->type);
	cbor_encode_unsigned_integer(&writer, (uint64_t)schema->unit);

	if (schema->range_min >= 0) {
		cbor_encode_unsigned_integer(&writer, (uint64_t)schema->range_min);
	} else {
		cbor_encode_negative_integer(&writer, schema->range_min);
	}

	if (schema->range_max >= 0) {
		cbor_encode_unsigned_integer(&writer, (uint64_t)schema->range_max);
	} else {
		cbor_encode_negative_integer(&writer, schema->range_max);
	}

	if (value >= 0) {
		cbor_encode_unsigned_integer(&writer, (uint64_t)value);
	} else {
		cbor_encode_negative_integer(&writer, value);
	}

	return cbor_writer_len(&writer) - len;
}
#else
size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value)
{
	size_t len = cbor_writer_len(&writer);

	cbor_encode_unsigned_integer(&writer, (uint64_t)key);

	if (value >= 0) {
		cbor_encode_unsigned_integer(&writer, (uint64_t)value);
	} else {
		cbor_encode_negative_integer(&writer, value);
	}

	return cbor_writer_len(&writer) - len;
}
#endif
