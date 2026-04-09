/*
 * SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"

extern "C" {
#include "libmcu/metrics.h"
#include "libmcu/metrics_overrides.h"
#include "libmcu/version.h"

const char *metrics_get_serial_number_string(void)
{
	return "SN01";
}

uint64_t metrics_get_unix_timestamp(void)
{
	return 1234567890ULL;
}

void metrics_lock(void)
{
}

void metrics_unlock(void)
{
}
}

static uint8_t libmcu_version_cbor_msb(void)
{
	return (uint8_t)((LIBMCU_VERSION >> 8) & 0xffU);
}

static uint8_t libmcu_version_cbor_lsb(void)
{
	return (uint8_t)(LIBMCU_VERSION & 0xffU);
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
		0x19, libmcu_version_cbor_msb(), libmcu_version_cbor_lsb(),
		0x1A, 0x49, 0x96, 0x02, 0xD2, /* ts = 1234567890 */
	};
	uint8_t buf[32] = { 0, };

	size_t written = metrics_collect(buf, sizeof(buf));

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
		0x19, libmcu_version_cbor_msb(), libmcu_version_cbor_lsb(),
		0x1A, 0x49, 0x96, 0x02, 0xD2, /* ts = 1234567890 */
		0x00,
		0x1A, 0x12, 0x34, 0x56, 0x78,
	};
	uint8_t buf[32] = { 0, };

	metrics_set(ReportInterval, 0x12345678);

	size_t written = metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(sizeof(expected), written);
	MEMCMP_EQUAL(expected, buf, written);
}

TEST(metrics_cbor, collect_ShouldReturnExactSize_WhenBufIsNull)
{
	uint8_t buf[32] = { 0, };

	metrics_set(ReportInterval, 1);
	metrics_set(WallTime, -1);

	size_t needed = metrics_collect(NULL, 0);
	size_t written = metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(written, needed);
}

TEST(metrics_cbor, count_ShouldExcludeMetadata)
{
	const size_t nr_metrics = 9U;

	LONGS_EQUAL(nr_metrics, metrics_count());
	metrics_set(ReportInterval, 1);
	LONGS_EQUAL(nr_metrics, metrics_count());
}
