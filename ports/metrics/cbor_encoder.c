/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/metrics.h"
#include "libmcu/metrics_overrides.h"
#include "libmcu/compiler.h"

#include "cbor/cbor.h"
#include "cbor/encoder.h"

#include <string.h>

enum {
	METRICS_CBOR_METADATA_KEY = -1,
};

/* The metadata payload is a fixed-order, append-only array encoded as:
 *   -1: [sn, ts, ver]
 * Do not reorder existing fields. Future metadata may only be appended. */
enum metrics_cbor_metadata_index {
	METRICS_CBOR_METADATA_SN = 0,
	METRICS_CBOR_METADATA_TS,
	METRICS_CBOR_METADATA_VER,
	METRICS_CBOR_METADATA_COUNT,
};

static_assert(METRICS_CBOR_METADATA_SN == 0,
		"CBOR metadata array order must start with SN");
static_assert(METRICS_CBOR_METADATA_TS == 1,
		"CBOR metadata array order must keep timestamp second");
static_assert(METRICS_CBOR_METADATA_VER == 2,
		"CBOR metadata array order must keep version third");
static_assert(METRICS_CBOR_METADATA_COUNT == 3,
		"CBOR metadata array is append-only; update decoder docs if extended");

static size_t cbor_encoded_uint_size(uint64_t value)
{
	if (value < 24) {
		return 1;
	} else if (value <= 0xffU) {
		return 2;
	} else if (value <= 0xffffU) {
		return 3;
	} else if (value <= 0xffffffffU) {
		return 5;
	}

	return 9;
}

static size_t cbor_encoded_int_size(int64_t value)
{
	if (value >= 0) {
		return cbor_encoded_uint_size((uint64_t)value);
	}

	return cbor_encoded_uint_size((uint64_t)(-1 - value));
}

static size_t cbor_encoded_text_size(const char *text)
{
	const size_t len = strlen(text);

	return cbor_encoded_uint_size((uint64_t)len) + len;
}

static size_t cbor_encoded_metric_value_size(int32_t value)
{
	return cbor_encoded_int_size((int64_t)value);
}

#if defined(METRICS_SCHEMA_IBS)
static size_t cbor_encoded_schema_value_size(const struct metric_schema *schema,
		int32_t value)
{
	return cbor_encoded_uint_size(5)
		+ cbor_encoded_uint_size((uint64_t)schema->type)
		+ cbor_encoded_uint_size((uint64_t)schema->unit)
		+ cbor_encoded_int_size((int64_t)schema->range_min)
		+ cbor_encoded_int_size((int64_t)schema->range_max)
		+ cbor_encoded_metric_value_size(value);
}
#endif

static size_t cbor_encoded_metadata_size(void)
{
	const char *sn = metrics_get_serial_number_string();
	const char *ver = metrics_get_version_string();

	if (sn == NULL) {
		sn = "";
	}

	if (ver == NULL) {
		ver = "";
	}

	return cbor_encoded_int_size(METRICS_CBOR_METADATA_KEY)
		+ cbor_encoded_uint_size(METRICS_CBOR_METADATA_COUNT)
		+ cbor_encoded_text_size(sn)
		+ cbor_encoded_uint_size(metrics_get_unix_timestamp())
		+ cbor_encoded_text_size(ver);
}

static cbor_error_t cbor_encode_metadata(cbor_writer_t *w)
{
	const char *sn = metrics_get_serial_number_string();
	const char *ver = metrics_get_version_string();

	if (sn == NULL) {
		sn = "";
	}

	if (ver == NULL) {
		ver = "";
	}

	if (cbor_encode_negative_integer(w, METRICS_CBOR_METADATA_KEY)
			!= CBOR_SUCCESS) {
		return CBOR_OVERRUN;
	}
	if (cbor_encode_array(w, METRICS_CBOR_METADATA_COUNT) != CBOR_SUCCESS) {
		return CBOR_OVERRUN;
	}
	if (cbor_encode_text_string(w, sn, strlen(sn)) != CBOR_SUCCESS) {
		return CBOR_OVERRUN;
	}
	if (cbor_encode_unsigned_integer(w, metrics_get_unix_timestamp())
			!= CBOR_SUCCESS) {
		return CBOR_OVERRUN;
	}
	if (cbor_encode_text_string(w, ver, strlen(ver)) != CBOR_SUCCESS) {
		return CBOR_OVERRUN;
	}

	return CBOR_SUCCESS;
}

size_t metrics_encode_header(void *buf, size_t bufsize,
		uint32_t nr_total, uint32_t nr_updated, void *ctx)
{
	unused(nr_total);

	if (buf == NULL) {
		return cbor_encoded_uint_size((uint64_t)(nr_updated + 1U))
			+ cbor_encoded_metadata_size();
	}

	if (ctx == NULL) {
		return 0;
	}

	cbor_writer_t *writer = (cbor_writer_t *)ctx;
	cbor_writer_init(writer, buf, bufsize);
	if (cbor_encode_map(writer, nr_updated + 1U) != CBOR_SUCCESS) {
		return 0;
	}
	if (cbor_encode_metadata(writer) != CBOR_SUCCESS) {
		return 0;
	}

	return cbor_writer_len(writer);
}

#if defined(METRICS_SCHEMA_IBS)
size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value,
		const struct metric_schema *schema, void *ctx)
{
	unused(buf);
	unused(bufsize);
	unused(ctx);

	if (buf == NULL) {
		return cbor_encoded_uint_size((uint64_t)key)
			+ cbor_encoded_schema_value_size(schema, value);
	}
	if (ctx == NULL) {
		return 0;
	}

	cbor_writer_t *writer = (cbor_writer_t *)ctx;
	size_t len = cbor_writer_len(writer);

	if (cbor_encode_unsigned_integer(writer, (uint64_t)key)
			!= CBOR_SUCCESS) {
		return 0;
	}

	/* value → [class, unit, range_min, range_max, value] */
	if (cbor_encode_array(writer, 5) != CBOR_SUCCESS) {
		return 0;
	}

	if (cbor_encode_unsigned_integer(writer, (uint64_t)schema->type)
			!= CBOR_SUCCESS) {
		return 0;
	}
	if (cbor_encode_unsigned_integer(writer, (uint64_t)schema->unit)
			!= CBOR_SUCCESS) {
		return 0;
	}

	if (schema->range_min >= 0) {
		if (cbor_encode_unsigned_integer(writer, (uint64_t)schema->range_min)
				!= CBOR_SUCCESS) {
			return 0;
		}
	} else {
		if (cbor_encode_negative_integer(writer, schema->range_min)
				!= CBOR_SUCCESS) {
			return 0;
		}
	}

	if (schema->range_max >= 0) {
		if (cbor_encode_unsigned_integer(writer, (uint64_t)schema->range_max)
				!= CBOR_SUCCESS) {
			return 0;
		}
	} else {
		if (cbor_encode_negative_integer(writer, schema->range_max)
				!= CBOR_SUCCESS) {
			return 0;
		}
	}

	if (value >= 0) {
		if (cbor_encode_unsigned_integer(writer, (uint64_t)value)
				!= CBOR_SUCCESS) {
			return 0;
		}
	} else {
		if (cbor_encode_negative_integer(writer, value) != CBOR_SUCCESS) {
			return 0;
		}
	}

	return cbor_writer_len(writer) - len;
}
#else
size_t metrics_encode_each(void *buf, size_t bufsize,
		metric_key_t key, int32_t value, void *ctx)
{
	unused(buf);
	unused(bufsize);
	unused(ctx);

	if (buf == NULL) {
		return cbor_encoded_uint_size((uint64_t)key)
			+ cbor_encoded_metric_value_size(value);
	}

	if (ctx == NULL) {
		return 0;
	}

	cbor_writer_t *writer = (cbor_writer_t *)ctx;
	size_t len = cbor_writer_len(writer);

	if (cbor_encode_unsigned_integer(writer, (uint64_t)key)
			!= CBOR_SUCCESS) {
		return 0;
	}

	if (value >= 0) {
		if (cbor_encode_unsigned_integer(writer, (uint64_t)value)
				!= CBOR_SUCCESS) {
			return 0;
		}
	} else {
		if (cbor_encode_negative_integer(writer, value)
				!= CBOR_SUCCESS) {
			return 0;
		}
	}

	return cbor_writer_len(writer) - len;
}
#endif
