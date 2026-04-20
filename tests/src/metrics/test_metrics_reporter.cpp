/*
 * SPDX-FileCopyrightText: 2026 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <string.h>
#include <errno.h>

extern "C" {
#include "libmcu/metrics_reporter.h"
#include "libmcu/metrics.h"
#include "libmcu/metrics_overrides.h"
#include "libmcu/metricfs.h"
}

static const uint8_t FAKE_PAYLOAD[] = { 0xDE, 0xAD, 0xBE, 0xEF };
static const size_t FAKE_PAYLOAD_LEN = sizeof(FAKE_PAYLOAD);

void metrics_report_prepare(void *ctx)
{
	mock().actualCall("prepare").withParameter("ctx", ctx);
}

int metrics_report_transmit(const void *data, size_t datasize, void *ctx)
{
	return mock().actualCall("transmit")
		.withParameter("data", const_cast<void *>(data))
		.withParameter("datasize", (int)datasize)
		.withParameter("ctx", ctx)
		.returnIntValue();
}

size_t metrics_collect(void *buf, const size_t bufsize)
{
	int rc = mock().actualCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)bufsize)
		.returnIntValue();
	if (rc > 0 && buf) {
		size_t copylen = (size_t)rc < bufsize ? (size_t)rc : bufsize;
		memcpy(buf, FAKE_PAYLOAD, copylen);
	}
	return (size_t)rc;
}

void metrics_reset(void)
{
	mock().actualCall("reset");
}

uint16_t metricfs_count(const struct metricfs *fs)
{
	return (uint16_t)mock().actualCall("mfs_count")
		.withParameter("fs", const_cast<struct metricfs *>(fs))
		.returnIntValue();
}

int metricfs_peek_first(struct metricfs *fs,
		void *buf, const size_t bufsize, metricfs_id_t *id)
{
	int rc = mock().actualCall("mfs_peek_first")
		.withParameter("fs", fs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)bufsize)
		.returnIntValue();
	if (rc > 0 && buf) {
		size_t copylen = (size_t)rc < bufsize ? (size_t)rc : bufsize;
		memcpy(buf, FAKE_PAYLOAD, copylen);
	}
	if (id) {
		*id = 0;
	}
	return rc;
}

int metricfs_del_first(struct metricfs *fs, metricfs_id_t *id)
{
	return mock().actualCall("mfs_del_first")
		.withParameter("fs", fs)
		.returnIntValue();
}

int metricfs_write(struct metricfs *fs,
		const void *data, const size_t datasize, metricfs_id_t *id)
{
	return mock().actualCall("mfs_write")
		.withParameter("fs", fs)
		.withParameter("data", const_cast<void *>(data))
		.withParameter("datasize", (int)datasize)
		.returnIntValue();
}

static int fake_mfs_backing;
static struct metricfs *mfs = (struct metricfs *)&fake_mfs_backing;
static uint8_t buf[64];

TEST_GROUP(MetricsReporter) {
	void setup(void) {
		memset(buf, 0, sizeof(buf));
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}

	/* helper: no store entries, collect returns payload, transmit succeeds */
	void expect_happy_path_no_store(void *ctx) {
		mock().expectOneCall("prepare").withParameter("ctx", ctx);
		mock().expectOneCall("mfs_count")
			.withParameter("fs", mfs).andReturnValue(0);
		mock().expectOneCall("collect")
			.withParameter("buf", buf)
			.withParameter("bufsize", (int)sizeof(buf))
			.andReturnValue((int)FAKE_PAYLOAD_LEN);
		mock().expectOneCall("transmit")
			.withParameter("data", buf)
			.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
			.withParameter("ctx", ctx)
			.andReturnValue(0);
		mock().expectOneCall("reset");
		mock().expectOneCall("mfs_count")
			.withParameter("fs", mfs).andReturnValue(0);
	}
};

/* =========================================================================
 * Basic functionality
 * ========================================================================= */

TEST(MetricsReporter, ShouldReturnZero_WhenTransmitSucceeds_NoStore) {
	expect_happy_path_no_store(NULL);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporter, ShouldCallPrepareBeforeCollect) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue(0);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporter, ShouldReturnZero_WhenNothingToCollectAndStoreEmpty) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue(0);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporter, ShouldResetMetrics_WhenTransmitSucceeds_FreshCollect) {
	expect_happy_path_no_store(NULL);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporter, ShouldPassUserContext_ToPrepareAndTransmit) {
	int user_ctx = 42;
	void *ctx = &user_ctx;

	expect_happy_path_no_store(ctx);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, ctx));
}

/* =========================================================================
 * mfs == NULL (no persistence)
 * ========================================================================= */

TEST(MetricsReporter, ShouldWork_WhenMfsIsNull) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(0);
	mock().expectOneCall("reset");
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), NULL, NULL));
}

TEST(MetricsReporter, ShouldNotResetAndReturnError_WhenTransmitFailsWithoutMfs) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(-EIO);
	LONGS_EQUAL(-EIO, metrics_report(buf, sizeof(buf), NULL, NULL));
}

/* =========================================================================
 * Store (metricfs) draining — sending old data from fs
 * ========================================================================= */

TEST(MetricsReporter, ShouldSendFromStore_WhenStoreHasEntries) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(0);
	mock().expectOneCall("mfs_del_first")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporter, ShouldReturnEAGAIN_WhenStoreStillHasEntries) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(2);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(0);
	mock().expectOneCall("mfs_del_first")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	LONGS_EQUAL(-EAGAIN, metrics_report(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporter, ShouldNotResetMemoryMetrics_WhenSendingFromStore) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(0);
	mock().expectOneCall("mfs_del_first")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporter, ShouldNotDeleteFromStore_WhenTransmitFailsOnStoredData) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(-EIO);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	LONGS_EQUAL(-EAGAIN, metrics_report(buf, sizeof(buf), mfs, NULL));
}

/* =========================================================================
 * Transmit failure with persistence
 * ========================================================================= */

TEST(MetricsReporter, ShouldPersistAndReset_WhenTransmitFailsWithMfs) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(-EIO);
	mock().expectOneCall("mfs_write")
		.withParameter("fs", mfs)
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.andReturnValue(0);
	mock().expectOneCall("reset");
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	LONGS_EQUAL(-EAGAIN, metrics_report(buf, sizeof(buf), mfs, NULL));
}

/* =========================================================================
 * BUG DETECTION: metricfs_write fails but metrics_reset still called
 * ========================================================================= */

TEST(MetricsReporter, ShouldNotReset_WhenBothTransmitAndPersistFail) {
	/* transmit 실패 + metricfs_write 실패 = 데이터가 어디에도 없음.
	 * reset하면 데이터 완전 유실. reset하지 않아야 다음 사이클에 재시도 가능. */
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(-EIO);
	mock().expectOneCall("mfs_write")
		.withParameter("fs", mfs)
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.andReturnValue(-ENOSPC);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	LONGS_EQUAL(-EIO, metrics_report(buf, sizeof(buf), mfs, NULL));
}

/* =========================================================================
 * Ordering / race scenarios
 * ========================================================================= */

TEST(MetricsReporter, ShouldDrainStoreBeforeCollectingFresh) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(0);
	mock().expectOneCall("mfs_del_first")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporter, ShouldFallbackToCollect_WhenPeekFirstFails) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue(-EIO);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(0);
	mock().expectOneCall("reset");
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	LONGS_EQUAL(-EAGAIN, metrics_report(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporter, ShouldFallbackToCollect_WhenPeekFirstReturnsZero) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue(0);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(0);
	mock().expectOneCall("reset");
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	LONGS_EQUAL(-EAGAIN, metrics_report(buf, sizeof(buf), mfs, NULL));
}

/* =========================================================================
 * Scenario: store에 있는 과거 메트릭 전송 중 현재 메모리 메트릭 리셋 여부
 * ========================================================================= */

TEST(MetricsReporter, ShouldNotResetCurrentMetrics_WhenStoredTransmitFails) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(-ETIMEDOUT);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	LONGS_EQUAL(-EAGAIN, metrics_report(buf, sizeof(buf), mfs, NULL));
}

/* =========================================================================
 * Scenario: metricfs_del_first fails after successful transmit
 * ========================================================================= */

TEST(MetricsReporter, ShouldReturnEAGAIN_WhenDelFirstFailsAfterStoredTransmit) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(0);
	mock().expectOneCall("mfs_del_first")
		.withParameter("fs", mfs)
		.andReturnValue(-EIO);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	LONGS_EQUAL(-EAGAIN, metrics_report(buf, sizeof(buf), mfs, NULL));
}

/* =========================================================================
 * Scenario: EAGAIN masking transmit errors
 * ========================================================================= */

TEST(MetricsReporter, ShouldReturnEAGAIN_NotTransmitError_WhenStoreHasEntries) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(-EIO);
	mock().expectOneCall("mfs_write")
		.withParameter("fs", mfs)
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.andReturnValue(0);
	mock().expectOneCall("reset");
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	LONGS_EQUAL(-EAGAIN, metrics_report(buf, sizeof(buf), mfs, NULL));
}

/* =========================================================================
 * Scenario: prepare side effects on store path
 * ========================================================================= */

TEST(MetricsReporter, ShouldCallPrepareEvenWhenSendingFromStore) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(0);
	mock().expectOneCall("mfs_del_first")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, NULL));
}

/* =========================================================================
 * Multi-cycle ordering scenario
 * ========================================================================= */

TEST(MetricsReporter, ShouldDrainAllStoredEntriesBeforeSendingFresh) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(0);
	mock().expectOneCall("mfs_del_first")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("transmit")
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.withParameter("ctx", (void *)NULL)
		.andReturnValue(0);
	mock().expectOneCall("reset");
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, NULL));
}
