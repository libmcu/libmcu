/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"

extern "C" {
#include "libmcu/metrics.h"
#include "libmcu/metrics_overrides.h"
#include "cbor/cbor.h"

const char *metrics_get_serial_number_string(void)
{
	return "SN01";
}

uint64_t metrics_get_unix_timestamp(void)
{
	return 1234567890ULL;
}

const char *metrics_get_version_string(void)
{
	return "v1.0.0";
}

void metrics_lock(void)
{
}

void metrics_unlock(void)
{
}
}

TEST_GROUP(metrics_cbor)
{
	void setup(void)
	{
		metrics_init(true);
	}

	void teardown(void)
	{
	}
};

TEST(metrics_cbor, collect_ShouldEncodeMetadataOnly_WhenNoMetricSet)
{
	const uint8_t expected[] = {
		0xA1,
		0x20,
		0x83,
		0x64, 'S', 'N', '0', '1',
		0x1A, 0x49, 0x96, 0x02, 0xD2, /* ts = 1234567890 */
		0x66, 'v', '1', '.', '0', '.', '0',
	};
	uint8_t buf[32] = { 0, };
	cbor_writer_t writer;

	size_t written = metrics_collect(buf, sizeof(buf), &writer);

	LONGS_EQUAL(sizeof(expected), written);
	MEMCMP_EQUAL(expected, buf, written);
}

TEST(metrics_cbor, collect_ShouldEncodeMetadataAndMetric_WhenMetricIsSet)
{
	const uint8_t expected[] = {
		0xA2,
		0x20,
		0x83,
		0x64, 'S', 'N', '0', '1',
		0x1A, 0x49, 0x96, 0x02, 0xD2, /* ts = 1234567890 */
		0x66, 'v', '1', '.', '0', '.', '0',
		0x00,
		0x1A, 0x12, 0x34, 0x56, 0x78,
	};
	uint8_t buf[32] = { 0, };
	cbor_writer_t writer;

	metrics_set(ReportInterval, 0x12345678);

	size_t written = metrics_collect(buf, sizeof(buf), &writer);

	LONGS_EQUAL(sizeof(expected), written);
	MEMCMP_EQUAL(expected, buf, written);
}

TEST(metrics_cbor, collect_ShouldReturnExactSize_WhenBufIsNull)
{
	uint8_t buf[32] = { 0, };
	cbor_writer_t writer;

	metrics_set(ReportInterval, 1);
	metrics_set(WallTime, -1);

	size_t needed = metrics_collect(NULL, 0, NULL);
	size_t written = metrics_collect(buf, sizeof(buf), &writer);

	LONGS_EQUAL(written, needed);
}

TEST(metrics_cbor, count_ShouldExcludeMetadata)
{
	const size_t nr_metrics = 11U;

	LONGS_EQUAL(nr_metrics, metrics_count());
	metrics_set(ReportInterval, 1);
	LONGS_EQUAL(nr_metrics, metrics_count());
}
