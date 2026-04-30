/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "libmcu/metrics.h"
#include <time.h>
#include "libmcu/logging.h"

#define SAVED_METRICS_LEN		3

static struct {
	metric_key_t id;
	int32_t value;
} saved_metrics[SAVED_METRICS_LEN];

static int callback_call_count;

static void count_callback(metric_key_t keyid, int32_t value, void *ctx)
{
	(void)keyid;
	(void)value;
	(void)ctx;
	callback_call_count++;
}

static void clear_saved_metrics(void)
{
	for (int i = 0; i < SAVED_METRICS_LEN; i++) {
		saved_metrics[i].id = (metric_key_t)0;
		saved_metrics[i].value = 0;
	}
}

static void save_metric_ReportInterval_only(metric_key_t keyid, int32_t value,
					    void *ctx)
{
	(void)ctx;
	if (keyid != ReportInterval) {
		return;
	}

	saved_metrics[0].id = keyid;
	saved_metrics[0].value = value;
}

static void print_metric_each(metric_key_t keyid, int32_t value, void *ctx)
{
	(void)ctx;
	debug("Metric %02u : %d", keyid, value);
}

static int32_t get_heap_hwm(void)
{
	return 1234;
}

static void report_periodic(void)
{
	metrics_set(HeapHighWaterMark, get_heap_hwm());

	static int32_t stamp;
	int32_t now = (int32_t)time(NULL);
	metrics_increase_by(ReportInterval, now - stamp);
	stamp = now;

	metrics_iterate(print_metric_each, 0);

	uint8_t buf[128];
	metrics_collect(buf, sizeof(buf), NULL);
	metrics_reset();
}

TEST_GROUP(metrics) {
	void setup(void) {
		clear_saved_metrics();
		callback_call_count = 0;
		metrics_init(true);
	}
	void teardown() {
	}
};

TEST(metrics, get_ShouldReturnValue) {
	int32_t expected = 12345;
	metrics_set(ReportInterval, expected);
	LONGS_EQUAL(expected, metrics_get(ReportInterval));
}

TEST(metrics, set_ShouldSetMetricValue123_WhenValue123Given) {
	metrics_iterate(save_metric_ReportInterval_only, 0);
	LONGS_EQUAL(ReportInterval, saved_metrics[0].id);
	LONGS_EQUAL(0, saved_metrics[0].value);

	metrics_set(ReportInterval, 123);
	metrics_iterate(save_metric_ReportInterval_only, 0);

	LONGS_EQUAL(ReportInterval, saved_metrics[0].id);
	LONGS_EQUAL(123, saved_metrics[0].value);
}

TEST(metrics, increase_ShouldIncreaseValueByOne) {
	metrics_iterate(save_metric_ReportInterval_only, 0);
	LONGS_EQUAL(ReportInterval, saved_metrics[0].id);
	LONGS_EQUAL(0, saved_metrics[0].value);

	metrics_increase(ReportInterval);
	metrics_iterate(save_metric_ReportInterval_only, 0);

	LONGS_EQUAL(ReportInterval, saved_metrics[0].id);
	LONGS_EQUAL(1, saved_metrics[0].value);
}

TEST(metrics, increase_by_ShouldIncreaseValueByN_WhenN45Given) {
	metrics_iterate(save_metric_ReportInterval_only, 0);
	LONGS_EQUAL(ReportInterval, saved_metrics[0].id);
	LONGS_EQUAL(0, saved_metrics[0].value);

	metrics_increase_by(ReportInterval, 45);
	metrics_iterate(save_metric_ReportInterval_only, 0);

	LONGS_EQUAL(ReportInterval, saved_metrics[0].id);
	LONGS_EQUAL(45, saved_metrics[0].value);
}

TEST(metrics, reset_ShouldResetAllMetrics) {
	metrics_set(ReportInterval, 1234);

	metrics_reset();
	metrics_iterate(save_metric_ReportInterval_only, 0);

	LONGS_EQUAL(ReportInterval, saved_metrics[0].id);
	LONGS_EQUAL(0, saved_metrics[0].value);
}

TEST(metrics, collect_ShouldReturnSizeOfAllEncodedMetrics_WhenZeroedValueGiven) {
	uint8_t expected_encoded_data[128] = { 0, };
	uint8_t buf[128];
	size_t size = metrics_collect(buf, sizeof(buf), NULL);
	LONGS_EQUAL(0, size);
	MEMCMP_EQUAL(expected_encoded_data, buf, size);
}

TEST(metrics, collect_ShouldReturnSizeOfAllEncodedMetrics_WhenReportIntervalValueSet) {
	uint8_t expected_encoded_data[128] = { 0, 0, 0, 0, 0x78, 0x56, 0x34, 0x12, };
	uint8_t buf[128];
	metrics_set(ReportInterval, 0x12345678);
	size_t size = metrics_collect(buf, sizeof(buf), NULL);
	LONGS_EQUAL(8, size);
	MEMCMP_EQUAL(expected_encoded_data, buf, size);
}

TEST(metrics, stringify_key_ShouldReturnString_WhenKeyGiven) {
	char const *p = metrics_stringify_key(ReportInterval);
	STRCMP_EQUAL("ReportInterval", p);
}

TEST(metrics, is_set_ShouldReturnTrue_WhenReportIntervalIsSet) {
	metrics_set(ReportInterval, 0);
	LONGS_EQUAL(true, metrics_is_set(ReportInterval));
}

TEST(metrics, is_set_ShouldReturnFalse_WhenReportIntervalIsNotSet) {
	LONGS_EQUAL(false, metrics_is_set(ReportInterval));
}

TEST(metrics, count_ShouldReturnNumberOfMetrics) {
	LONGS_EQUAL(9, metrics_count());
}

TEST(metrics, set_if_min_ShouldSetMinValue_WhenNotSet) {
	metrics_set_if_min(ReportInterval, 123);
	LONGS_EQUAL(123, metrics_get(ReportInterval));
}

TEST(metrics, set_if_min_ShouldSetMinValue_WhenSmallerValueGiven) {
	metrics_set_if_min(ReportInterval, 123);
	metrics_set_if_min(ReportInterval, 122);
	LONGS_EQUAL(122, metrics_get(ReportInterval));
}

TEST(metrics, set_if_min_ShouldNotSetMinValue_WhenLargerValueGiven) {
	metrics_set_if_min(ReportInterval, 123);
	metrics_set_if_min(ReportInterval, 124);
	LONGS_EQUAL(123, metrics_get(ReportInterval));
}

TEST(metrics, set_if_min_ShouldNotSetMinValue_WhenSameValueGiven) {
	metrics_set_if_min(ReportInterval, 123);
	metrics_set_if_min(ReportInterval, 123);
	LONGS_EQUAL(123, metrics_get(ReportInterval));
}

TEST(metrics, set_if_max_ShouldSetMaxValue_WhenNotSet) {
	metrics_set_if_max(ReportInterval, 123);
	LONGS_EQUAL(123, metrics_get(ReportInterval));
}

TEST(metrics, set_if_max_ShouldSetMaxValue_WhenLargerValueGiven) {
	metrics_set_if_max(ReportInterval, 123);
	metrics_set_if_max(ReportInterval, 124);
	LONGS_EQUAL(124, metrics_get(ReportInterval));
}

TEST(metrics, set_if_max_ShouldNotSetMaxValue_WhenSmallerValueGiven) {
	metrics_set_if_max(ReportInterval, 123);
	metrics_set_if_max(ReportInterval, 122);
	LONGS_EQUAL(123, metrics_get(ReportInterval));
}

TEST(metrics, set_if_max_ShouldNotSetMaxValue_WhenSameValueGiven) {
	metrics_set_if_max(ReportInterval, 123);
	metrics_set_if_max(ReportInterval, 123);
	LONGS_EQUAL(123, metrics_get(ReportInterval));
}

TEST(metrics, unset_ShouldResetMetric_WhenMetricIsSet) {
	metrics_set(ReportInterval, 123);
	LONGS_EQUAL(true, metrics_is_set(ReportInterval));
	LONGS_EQUAL(123, metrics_get(ReportInterval));

	metrics_unset(ReportInterval);
	
	LONGS_EQUAL(false, metrics_is_set(ReportInterval));
	LONGS_EQUAL(0, metrics_get(ReportInterval));
}

TEST(metrics, unset_ShouldDoNothing_WhenMetricIsNotSet) {
	LONGS_EQUAL(false, metrics_is_set(ReportInterval));
	
	metrics_unset(ReportInterval);
	
	LONGS_EQUAL(false, metrics_is_set(ReportInterval));
	LONGS_EQUAL(0, metrics_get(ReportInterval));
}

TEST(metrics, test) {
	metrics_set(WallTime, 10);
	// 1. metrics_init()
	// 2. timer_create(report_periodic, 1hour)
	report_periodic();
}

TEST(metrics, init_ShouldPreserveMetrics_WhenForceIsFalseAndMetricsValid) {
	// Initialize and set up valid metrics
	metrics_init(true);
	metrics_set(ReportInterval, 12345);
	metrics_set(HeapHighWaterMark, 67890);

	// Call init with force=false - should preserve existing valid metrics
	metrics_init(false);

	// Verify metrics were preserved (validation passed, no reinitialization)
	LONGS_EQUAL(12345, metrics_get(ReportInterval));
	LONGS_EQUAL(67890, metrics_get(HeapHighWaterMark));
	LONGS_EQUAL(true, metrics_is_set(ReportInterval));
	LONGS_EQUAL(true, metrics_is_set(HeapHighWaterMark));
}

TEST(metrics, init_ShouldResetMetrics_WhenForceIsTrue) {
	// Initialize and set some metrics
	metrics_init(true);
	metrics_set(ReportInterval, 12345);
	metrics_set(HeapHighWaterMark, 67890);

	// Verify metrics are set
	LONGS_EQUAL(12345, metrics_get(ReportInterval));
	LONGS_EQUAL(true, metrics_is_set(ReportInterval));

	// Call init with force=true - should reset all metrics
	metrics_init(true);

	// Verify all metrics were reset
	LONGS_EQUAL(0, metrics_get(ReportInterval));
	LONGS_EQUAL(0, metrics_get(HeapHighWaterMark));
	LONGS_EQUAL(false, metrics_is_set(ReportInterval));
	LONGS_EQUAL(false, metrics_is_set(HeapHighWaterMark));
}

TEST(metrics, init_ShouldInitializeAllMetricsToZero_WhenForceIsTrue) {
	// Force initialization
	metrics_init(true);

	// Iterate and verify all metrics are initialized to 0 and not set
	static int count = 0;
	count = 0;

	auto verify_unset = [](metric_key_t keyid, int32_t value, void *ctx) {
		int *counter = (int *)ctx;
		(*counter)++;
		LONGS_EQUAL(0, value);
	};

	metrics_iterate(verify_unset, &count);

	// No metrics should be set initially, so count should be 0
	LONGS_EQUAL(0, count);

	// Verify metrics count is correct
	LONGS_EQUAL(9, metrics_count());
}

TEST(metrics, set_max_min_ShouldSetBothMaxAndMin_WhenNotSet) {
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 100);
	LONGS_EQUAL(100, metrics_get(HeapHighWaterMark));
	LONGS_EQUAL(100, metrics_get(ReportInterval));
}

TEST(metrics, set_max_min_ShouldUpdateMax_WhenLargerValueGiven) {
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 100);
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 150);

	LONGS_EQUAL(150, metrics_get(HeapHighWaterMark));
	LONGS_EQUAL(100, metrics_get(ReportInterval));
}

TEST(metrics, set_max_min_ShouldUpdateMin_WhenSmallerValueGiven) {
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 100);
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 50);

	LONGS_EQUAL(100, metrics_get(HeapHighWaterMark));
	LONGS_EQUAL(50, metrics_get(ReportInterval));
}

TEST(metrics, set_max_min_ShouldNotUpdate_WhenMiddleValueGiven) {
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 100);
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 200);
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 150);

	// Max should remain 200, min should remain 100
	LONGS_EQUAL(200, metrics_get(HeapHighWaterMark));
	LONGS_EQUAL(100, metrics_get(ReportInterval));
}

TEST(metrics, set_max_min_ShouldHandleSequenceCorrectly) {
	// Set initial value
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 100);
	LONGS_EQUAL(100, metrics_get(HeapHighWaterMark));
	LONGS_EQUAL(100, metrics_get(ReportInterval));

	// Update with larger value - max updates
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 200);
	LONGS_EQUAL(200, metrics_get(HeapHighWaterMark));
	LONGS_EQUAL(100, metrics_get(ReportInterval));

	// Update with smaller value - min updates
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 50);
	LONGS_EQUAL(200, metrics_get(HeapHighWaterMark));
	LONGS_EQUAL(50, metrics_get(ReportInterval));

	// Update with middle value - no change
	metrics_set_max_min(HeapHighWaterMark, ReportInterval, 150);
	LONGS_EQUAL(200, metrics_get(HeapHighWaterMark));
	LONGS_EQUAL(50, metrics_get(ReportInterval));
}

/* iterate: 콜백 호출 횟수 검증 */

TEST(metrics, iterate_ShouldNotCallCallback_WhenNoMetricsAreSet) {
	/* setup에서 metrics_init(true) → 모든 메트릭 is_set=false */
	metrics_iterate(count_callback, NULL);
	LONGS_EQUAL(0, callback_call_count);
}

TEST(metrics, iterate_ShouldCallCallbackExactlyNTimes_WhenNMetricsAreSet) {
	metrics_set(ReportInterval, 1);
	metrics_set(WallTime, 2);
	metrics_set(BatteryPct, 50);

	metrics_iterate(count_callback, NULL);

	LONGS_EQUAL(3, callback_call_count);
}

TEST(metrics, iterate_ShouldNotCallCallback_AfterReset) {
	metrics_set(ReportInterval, 1);
	metrics_reset();

	metrics_iterate(count_callback, NULL);

	LONGS_EQUAL(0, callback_call_count);
}

/* collect: 버퍼 크기 경계 */

TEST(metrics, collect_ShouldReturnZero_WhenBufferIsSmallerThanOneEntry) {
	/* 기본 인코더: 1 entry = 4B key + 4B value = 8B */
	metrics_set(ReportInterval, 100);

	uint8_t buf[7]; /* 8B 미만 → encode_each가 0 반환 */
	size_t size = metrics_collect(buf, sizeof(buf), NULL);

	LONGS_EQUAL(0, size);
}

TEST(metrics, collect_ShouldEncodeEntry_WhenBufferIsExactlyOneEntrySize) {
	metrics_set(ReportInterval, 0);

	uint8_t buf[8];
	size_t size = metrics_collect(buf, sizeof(buf), NULL);

	LONGS_EQUAL(8, size);
}

/* collect: NULL buf dry-run (V1) */

TEST(metrics, collect_ShouldReturnNeededSize_WhenBufIsNull) {
	metrics_set(ReportInterval, 1);
	metrics_set(WallTime, 2);

	size_t needed = metrics_collect(NULL, 0, NULL);
	uint8_t buf[64];
	size_t written = metrics_collect(buf, sizeof(buf), NULL);

	LONGS_EQUAL(needed, written);
}

/* collect: 잘못된 key는 무시 (V2) */

TEST(metrics, set_ShouldDoNothing_WhenInvalidKeyGiven) {
	const metric_key_t invalid = (metric_key_t)metrics_count();
	metrics_set(invalid, 999);
	/* assert가 발동되지 않고 크래시 없이 반환되어야 한다 */
	LONGS_EQUAL(false, metrics_is_set(invalid));
}

/* count: 총 선언 수 반환 — set 여부와 무관 */

TEST(metrics, count_ShouldReturnTotalDeclaredCount_RegardlessOfSetState) {
	size_t total = metrics_count();

	metrics_set(ReportInterval, 1);
	LONGS_EQUAL(total, metrics_count());

	metrics_reset();
	LONGS_EQUAL(total, metrics_count());
}

/* increase_by: 음수로 감소 가능 */

TEST(metrics, increase_by_ShouldDecreaseValue_WhenNegativeAmountGiven) {
	metrics_set(ReportInterval, 100);
	metrics_increase_by(ReportInterval, -30);
	LONGS_EQUAL(70, metrics_get(ReportInterval));
}

TEST_GROUP(metrics_types) {
	void setup() {
		callback_call_count = 0;
		metrics_init(true);
	}
	void teardown() {
	}
};

/* stringify */

TEST(metrics_types, stringify_ShouldReturnKeyName_ForCounter) {
	STRCMP_EQUAL("UnexpectedRebootCount",
		metrics_stringify_key(UnexpectedRebootCount));
}

TEST(metrics_types, stringify_ShouldReturnKeyName_ForGauge) {
	STRCMP_EQUAL("WallTime", metrics_stringify_key(WallTime));
}

TEST(metrics_types, stringify_ShouldReturnKeyName_ForPercentage) {
	STRCMP_EQUAL("BatteryPct", metrics_stringify_key(BatteryPct));
}

TEST(metrics_types, stringify_ShouldReturnKeyName_ForTimer) {
	STRCMP_EQUAL("ServerConnectedTime",
		metrics_stringify_key(ServerConnectedTime));
}

TEST(metrics_types, stringify_ShouldReturnKeyName_ForBytes) {
	STRCMP_EQUAL("HeapHighWaterMark",
		metrics_stringify_key(HeapHighWaterMark));
}

/* counter */

TEST(metrics_types, counter_ShouldAccumulateViaIncrease) {
	metrics_increase(UnexpectedRebootCount);
	metrics_increase(UnexpectedRebootCount);
	metrics_increase(UnexpectedRebootCount);
	LONGS_EQUAL(3, metrics_get(UnexpectedRebootCount));
}

/* gauge */

TEST(metrics_types, gauge_ShouldSetAndGetArbitraryValue) {
	metrics_set(WallTime, 1700000000);
	LONGS_EQUAL(1700000000, metrics_get(WallTime));
}

TEST(metrics_types, gauge_ShouldTrackMinMax_ViaSetMaxMin) {
	metrics_set_max_min(HeapHighWaterMark, StackHighWaterMark, 500);
	metrics_set_max_min(HeapHighWaterMark, StackHighWaterMark, 900);
	metrics_set_max_min(HeapHighWaterMark, StackHighWaterMark, 200);
	LONGS_EQUAL(900, metrics_get(HeapHighWaterMark));
	LONGS_EQUAL(200, metrics_get(StackHighWaterMark));
}

/* percentage */

TEST(metrics_types, percentage_ShouldSetValueInRange) {
	metrics_set(BatteryPct, 0);
	LONGS_EQUAL(0, metrics_get(BatteryPct));

	metrics_set(BatteryPct, 50);
	LONGS_EQUAL(50, metrics_get(BatteryPct));

	metrics_set(BatteryPct, 100);
	LONGS_EQUAL(100, metrics_get(BatteryPct));
}

/* timer */

TEST(metrics_types, timer_ShouldAccumulateElapsed_ViaIncreaseBy) {
	metrics_increase_by(ServerConnectedTime, 60);
	metrics_increase_by(ServerConnectedTime, 120);
	LONGS_EQUAL(180, metrics_get(ServerConnectedTime));
}

/* bytes */

TEST(metrics_types, bytes_ShouldTrackLowWatermark_ViaSetIfMin) {
	metrics_set_if_min(StackHighWaterMark, 4096);
	metrics_set_if_min(StackHighWaterMark, 2048);
	metrics_set_if_min(StackHighWaterMark, 3072);
	LONGS_EQUAL(2048, metrics_get(StackHighWaterMark));
}

TEST(metrics_types, bytes_ShouldTrackHighWatermark_ViaSetIfMax) {
	metrics_set_if_max(HeapHighWaterMark, 1024);
	metrics_set_if_max(HeapHighWaterMark, 8192);
	metrics_set_if_max(HeapHighWaterMark, 4096);
	LONGS_EQUAL(8192, metrics_get(HeapHighWaterMark));
}

/* count */

TEST(metrics_types, count_ShouldIncludeAllTypedMetrics) {
	LONGS_EQUAL(9, metrics_count());
}

/* metrics_set_pct */

TEST(metrics_types, pct_ShouldComputeCorrectPercentage) {
	metrics_set_pct(BatteryPct, 3, 4);
	LONGS_EQUAL(75, metrics_get(BatteryPct));
}

TEST(metrics_types, pct_ShouldAvoidOverflow_WhenLargeNumeratorGiven) {
	/* num * 100 이 int32_t 범위를 초과하는 경우 */
	metrics_set_pct(BatteryPct, 1000000, 2000000);
	LONGS_EQUAL(50, metrics_get(BatteryPct));
}

TEST(metrics_types, pct_ShouldDoNothing_WhenDenomIsZero) {
	metrics_set_pct(BatteryPct, 1, 0);
	LONGS_EQUAL(false, metrics_is_set(BatteryPct));
}

/* OBS 설계 결정: 타입 제약은 런타임에 강제되지 않는다.
 * 서버가 스키마를 기준으로 유효성을 검사한다. */

TEST(metrics_types, percentage_ShouldAcceptOutOfRangeValue_BecauseRuntimeEnforcementIsServerSide) {
	metrics_set(BatteryPct, 150); /* 선언 범위 0–100 초과 */
	LONGS_EQUAL(150, metrics_get(BatteryPct));
}

TEST(metrics_types, gauge_ShouldAcceptValueOutsideRange_BecauseRuntimeEnforcementIsServerSide) {
	metrics_set(WallTime, -1); /* 선언 범위 GAUGE(0, INT32_MAX) 미만 */
	LONGS_EQUAL(-1, metrics_get(WallTime));
}

TEST(metrics_types, counter_ShouldAllowDecrement_ViaIncreaseByNegative) {
	/* COUNTER는 단조 증가를 의도하나 런타임 강제 없음 */
	metrics_set(UnexpectedRebootCount, 10);
	metrics_increase_by(UnexpectedRebootCount, -3);
	LONGS_EQUAL(7, metrics_get(UnexpectedRebootCount));
}

TEST(metrics_types, counter_ShouldAllowArbitrarySetValue_BecauseRuntimeEnforcementIsServerSide) {
	/* metrics_set()은 타입 의미론을 강제하지 않는다 */
	metrics_set(UnexpectedRebootCount, 100);
	metrics_set(UnexpectedRebootCount, 50); /* 감소 */
	LONGS_EQUAL(50, metrics_get(UnexpectedRebootCount));
}

/* iterate: typed 메트릭만 set 시 콜백 정확히 N회 */

TEST(metrics_types, iterate_ShouldCallCallbackForEachTypedMetricSet) {
	metrics_set(UnexpectedRebootCount, 1);
	metrics_set(BatteryPct, 80);

	metrics_iterate(count_callback, NULL);

	LONGS_EQUAL(2, callback_call_count);
}
