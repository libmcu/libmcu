/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * IBS 모드 스키마 인코딩 검증 테스트.
 * -DMETRICS_SCHEMA_IBS 플래그로 빌드된 경우에만 의미 있다.
 *
 * 기본 IBS 페이로드 레이아웃 (little-endian):
 *   [2B: nr_updated]
 *   × nr_updated:
 *     [4B: key][1B: class][1B: unit][4B: range_min][4B: range_max][4B: value]
 */

#include "CppUTest/TestHarness.h"
#include "libmcu/metrics.h"
#include "libmcu/metrics_schema.h"
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define IBS_HEADER_SIZE		2
#define IBS_ENTRY_SIZE		18	/* 4+1+1+4+4+4 */
#define BUF_SIZE		256

static uint8_t buf[BUF_SIZE];

/* helpers ------------------------------------------------------------------ */

static uint16_t read_u16(const uint8_t *b, size_t off)
{
	uint16_t v;
	memcpy(&v, b + off, sizeof(v));
	return v;
}

static uint32_t read_u32(const uint8_t *b, size_t off)
{
	uint32_t v;
	memcpy(&v, b + off, sizeof(v));
	return v;
}

static int32_t read_i32(const uint8_t *b, size_t off)
{
	int32_t v;
	memcpy(&v, b + off, sizeof(v));
	return v;
}

static const uint8_t *entry_base(const uint8_t *b, int idx)
{
	return b + IBS_HEADER_SIZE + idx * IBS_ENTRY_SIZE;
}

static uint16_t nr_updated(const uint8_t *b)
{
	return read_u16(b, 0);
}

static uint32_t entry_key(const uint8_t *b, int idx)
{
	return read_u32(entry_base(b, idx), 0);
}

static uint8_t entry_class(const uint8_t *b, int idx)
{
	return entry_base(b, idx)[4];
}

static uint8_t entry_unit(const uint8_t *b, int idx)
{
	return entry_base(b, idx)[5];
}

static int32_t entry_range_min(const uint8_t *b, int idx)
{
	return read_i32(entry_base(b, idx), 6);
}

static int32_t entry_range_max(const uint8_t *b, int idx)
{
	return read_i32(entry_base(b, idx), 10);
}

static int32_t entry_value(const uint8_t *b, int idx)
{
	return read_i32(entry_base(b, idx), 14);
}

/* test group --------------------------------------------------------------- */

TEST_GROUP(metrics_schema)
{
	void setup(void)
	{
		memset(buf, 0, sizeof(buf));
		metrics_init(true);
	}
	void teardown(void) {}
};

/*
 * 아무 메트릭도 set하지 않으면 헤더(2B)만 기록되고 nr_updated == 0.
 */
TEST(metrics_schema, empty_payload_when_no_metrics_set)
{
	size_t len = metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(IBS_HEADER_SIZE, len);
	LONGS_EQUAL(0, nr_updated(buf));
}

/*
 * 헤더의 nr_updated는 실제로 set된 메트릭 수와 일치해야 한다.
 */
TEST(metrics_schema, nr_updated_matches_set_count)
{
	metrics_set(BatteryPct, 80);
	metrics_set(UnexpectedRebootCount, 2);
	metrics_set(ServerConnectedTime, 60);
	metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(3, nr_updated(buf));
}

/*
 * buf == NULL dry-run이 반환하는 크기는 실제 encode 크기와 같아야 한다.
 * 또한 크기는 header + entry_size × nr_set 이어야 한다.
 */
TEST(metrics_schema, dry_run_size_matches_actual_and_formula)
{
	metrics_set(BatteryPct, 50);
	metrics_set(WallTime, 999);

	size_t dry_run = metrics_collect(NULL, 0);
	size_t actual  = metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(dry_run, actual);
	LONGS_EQUAL(IBS_HEADER_SIZE + 2 * IBS_ENTRY_SIZE, actual);
}

/*
 * 페이로드에는 set된 메트릭만 포함되어야 한다.
 * 7개 중 1개만 set했을 때 entry가 1개이고 key가 일치하는지 확인.
 */
TEST(metrics_schema, only_set_metric_appears_in_payload)
{
	metrics_set(BatteryPct, 42);

	size_t len = metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(IBS_HEADER_SIZE + 1 * IBS_ENTRY_SIZE, len);
	LONGS_EQUAL(1, nr_updated(buf));
	LONGS_EQUAL(BatteryPct, entry_key(buf, 0));
}

/*
 * COUNTER: class == METRIC_CLASS_COUNTER, unit == NONE, range [0, INT32_MAX].
 * 값도 그대로 인코딩되어야 한다.
 */
TEST(metrics_schema, counter_schema_and_value)
{
	metrics_set(UnexpectedRebootCount, 7);
	metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(METRIC_CLASS_COUNTER, entry_class(buf, 0));
	LONGS_EQUAL(METRIC_UNIT_NONE,     entry_unit(buf, 0));
	LONGS_EQUAL(0,                    entry_range_min(buf, 0));
	LONGS_EQUAL(INT32_MAX,            entry_range_max(buf, 0));
	LONGS_EQUAL(7,                    entry_value(buf, 0));
}

/*
 * GAUGE: class == METRIC_CLASS_GAUGE, range는 .def에서 지정한 값이어야 한다.
 * WallTime은 GAUGE(0, INT32_MAX)로 정의되어 있다.
 */
TEST(metrics_schema, gauge_schema_reflects_user_defined_range)
{
	metrics_set(WallTime, 1000);
	metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(METRIC_CLASS_GAUGE, entry_class(buf, 0));
	LONGS_EQUAL(METRIC_UNIT_NONE,   entry_unit(buf, 0));
	LONGS_EQUAL(0,                  entry_range_min(buf, 0));
	LONGS_EQUAL(INT32_MAX,          entry_range_max(buf, 0));
}

/*
 * PERCENTAGE: class == METRIC_CLASS_PERCENTAGE, range는 반드시 [0, 100].
 * 타입 정의에 따른 묵시적 범위이지 사용자 입력이 아니다.
 */
TEST(metrics_schema, percentage_schema_has_fixed_0_to_100_range)
{
	metrics_set(BatteryPct, 75);
	metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(METRIC_CLASS_PERCENTAGE, entry_class(buf, 0));
	LONGS_EQUAL(METRIC_UNIT_NONE,        entry_unit(buf, 0));
	LONGS_EQUAL(0,                       entry_range_min(buf, 0));
	LONGS_EQUAL(100,                     entry_range_max(buf, 0));
	LONGS_EQUAL(75,                      entry_value(buf, 0));
}

/*
 * TIMER: class == METRIC_CLASS_TIMER, unit은 .def에서 지정한 단위여야 한다.
 * ServerConnectedTime은 TIMER(s)로 정의되어 있다.
 */
TEST(metrics_schema, timer_schema_has_correct_class_and_unit)
{
	metrics_set(ServerConnectedTime, 120);
	metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(METRIC_CLASS_TIMER, entry_class(buf, 0));
	LONGS_EQUAL(METRIC_UNIT_S,      entry_unit(buf, 0));
	LONGS_EQUAL(0,                  entry_range_min(buf, 0));
}

/*
 * BYTES: class == METRIC_CLASS_BYTES, unit == NONE, range [0, INT32_MAX].
 */
TEST(metrics_schema, bytes_schema)
{
	metrics_set(StackHighWaterMark, 512);
	metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(METRIC_CLASS_BYTES, entry_class(buf, 0));
	LONGS_EQUAL(METRIC_UNIT_NONE,   entry_unit(buf, 0));
	LONGS_EQUAL(0,                  entry_range_min(buf, 0));
	LONGS_EQUAL(INT32_MAX,          entry_range_max(buf, 0));
}

/*
 * BINARY: class == METRIC_CLASS_BINARY, unit == NONE, range [0, 1].
 */
TEST(metrics_schema, binary_schema_has_fixed_0_to_1_range)
{
	metrics_set(EmergencyActive, 1);
	metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(METRIC_CLASS_BINARY, entry_class(buf, 0));
	LONGS_EQUAL(METRIC_UNIT_NONE,    entry_unit(buf, 0));
	LONGS_EQUAL(0,                   entry_range_min(buf, 0));
	LONGS_EQUAL(1,                   entry_range_max(buf, 0));
	LONGS_EQUAL(1,                   entry_value(buf, 0));
}

/*
 * STATE: class == METRIC_CLASS_STATE, unit == NONE, range is full int32_t.
 */
TEST(metrics_schema, state_schema_has_full_int32_range)
{
	metrics_set(RunnerState, 3);
	metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(METRIC_CLASS_STATE, entry_class(buf, 0));
	LONGS_EQUAL(METRIC_UNIT_NONE,   entry_unit(buf, 0));
	LONGS_EQUAL(INT32_MIN,          entry_range_min(buf, 0));
	LONGS_EQUAL(INT32_MAX,          entry_range_max(buf, 0));
	LONGS_EQUAL(3,                  entry_value(buf, 0));
}

/*
 * UNTYPED: class == METRIC_CLASS_UNTYPED, range는 int32_t 전체 [INT32_MIN, INT32_MAX].
 */
TEST(metrics_schema, untyped_schema_has_full_int32_range)
{
	metrics_set(ReportInterval, 30);
	metrics_collect(buf, sizeof(buf));

	LONGS_EQUAL(METRIC_CLASS_UNTYPED, entry_class(buf, 0));
	LONGS_EQUAL(METRIC_UNIT_NONE,     entry_unit(buf, 0));
	LONGS_EQUAL(INT32_MIN,            entry_range_min(buf, 0));
	LONGS_EQUAL(INT32_MAX,            entry_range_max(buf, 0));
}

/*
 * 버퍼가 부족하면 encode_each가 0을 반환하고 해당 항목은 기록되지 않는다.
 * header(2B) + entry 1개(18B) = 20B. 버퍼를 19B로 제한하면 entry는 들어가지 않는다.
 */
TEST(metrics_schema, entry_not_written_when_buffer_too_small)
{
	metrics_set(BatteryPct, 50);

	size_t len = metrics_collect(buf, IBS_HEADER_SIZE + IBS_ENTRY_SIZE - 1);

	LONGS_EQUAL(IBS_HEADER_SIZE, len);
}
