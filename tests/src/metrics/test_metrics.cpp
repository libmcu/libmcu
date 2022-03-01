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
	metrics_collect(buf, sizeof(buf));
	metrics_reset();
}

TEST_GROUP(metrics) {
	void setup(void) {
		clear_saved_metrics();
		metrics_init();
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
	size_t size = metrics_collect(buf, sizeof(buf));
	LONGS_EQUAL(28, size);
	MEMCMP_EQUAL(expected_encoded_data, buf, size);
}

TEST(metrics, collect_ShouldReturnSizeOfAllEncodedMetrics_WhenReportIntervalValueSet) {
	uint8_t expected_encoded_data[128] = { 0x78, 0x56, 0x34, 0x12, };
	uint8_t buf[128];
	metrics_set(ReportInterval, 0x12345678);
	size_t size = metrics_collect(buf, sizeof(buf));
	LONGS_EQUAL(28, size);
	MEMCMP_EQUAL(expected_encoded_data, buf, size);
}

TEST(metrics, stringify_key_ShouldReturnString_WhenKeyGiven) {
	char const *p = metrics_stringify_key(ReportInterval);
	STRCMP_EQUAL("ReportInterval", p);
}

TEST(metrics, test) {
	metrics_set(WallTime, 10);
	// 1. metrics_init()
	// 2. timer_create(report_periodic, 1hour)
	report_periodic();
}
