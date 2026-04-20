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

uint64_t metrics_get_unix_timestamp(void)
{
	return (uint64_t)mock().actualCall("get_timestamp")
		.returnUnsignedLongIntValue();
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

/* =========================================================================
 * Input validation
 * ========================================================================= */

TEST(MetricsReporter, ShouldReturnEINVAL_WhenBufIsNull) {
	LONGS_EQUAL(-EINVAL, metrics_report(NULL, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporter, ShouldReturnEINVAL_WhenBufsizeIsZero) {
	LONGS_EQUAL(-EINVAL, metrics_report(buf, 0, mfs, NULL));
}

/* =========================================================================
 * Buffer overflow guard
 * ========================================================================= */

TEST(MetricsReporter, ShouldReturnENOBUFS_WhenCollectExceedsBufsize) {
	uint8_t small_buf[1] = { 0 };

	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("collect")
		.withParameter("buf", small_buf)
		.withParameter("bufsize", (int)sizeof(small_buf))
		.andReturnValue((int)sizeof(small_buf) + 1);
	LONGS_EQUAL(-ENOBUFS, metrics_report(small_buf, sizeof(small_buf),
			mfs, NULL));
}

TEST(MetricsReporter, ShouldReturnENOBUFS_WhenPeekFirstExceedsBufsize) {
	uint8_t small_buf[1] = { 0 };

	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("mfs_peek_first")
		.withParameter("fs", mfs)
		.withParameter("buf", small_buf)
		.withParameter("bufsize", (int)sizeof(small_buf))
		.andReturnValue((int)sizeof(small_buf) + 1);
	LONGS_EQUAL(-ENOBUFS, metrics_report(small_buf, sizeof(small_buf),
			mfs, NULL));
}

TEST(MetricsReporter, ShouldCallPrepareBeforeCollect) {
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue(0);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
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
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	LONGS_EQUAL(0, metrics_report(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporter, ShouldReturnEAGAIN_WhenCollectReturnsZeroButStoreHasEntries) {
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
		.andReturnValue(0);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	LONGS_EQUAL(-EAGAIN, metrics_report(buf, sizeof(buf), mfs, NULL));
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
	/* transmit fail + metricfs_write fail = data exists nowhere.
	 * Resetting would lose data entirely; skip reset so next cycle can retry. */
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
 * Scenario: whether in-memory metrics are reset while sending stored entries
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

TEST(MetricsReporter, ShouldReturnDelError_WhenDelFirstFailsAfterStoredTransmit) {
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
	/* del_first failure returns error immediately — no count check */
	LONGS_EQUAL(-EIO, metrics_report(buf, sizeof(buf), mfs, NULL));
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

/* =========================================================================
 * metrics_report_periodic()
 * ========================================================================= */

TEST_GROUP(MetricsReporterPeriodic) {
	void setup(void) {
		memset(buf, 0, sizeof(buf));
		metrics_report_periodic_reset();
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(MetricsReporterPeriodic, ShouldReport_WhenCalledFirstTime) {
	mock().expectOneCall("get_timestamp").andReturnValue(1000UL);
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporterPeriodic, ShouldNotAdvanceLastReportTime_WhenDrainingBacklog) {
	/* Cycle 1 at t=1000: transmit fails → persisted, -EAGAIN */
	mock().expectOneCall("get_timestamp").andReturnValue(1000UL);
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
	LONGS_EQUAL(-EAGAIN, metrics_report_periodic(buf, sizeof(buf),
			mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	/* Cycle 2 at t=2000: drain one entry, backlog remains (-EAGAIN).
	 * last_report_time must NOT advance to 2000. */
	mock().expectOneCall("get_timestamp").andReturnValue(2000UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
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
	LONGS_EQUAL(-EAGAIN, metrics_report_periodic(buf, sizeof(buf),
			mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	/* Cycle 3 at t=3000: drain again, backlog remains (-EAGAIN).
	 * last_report_time still NOT advanced. */
	mock().expectOneCall("get_timestamp").andReturnValue(3000UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
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
		.withParameter("fs", mfs).andReturnValue(1);
	LONGS_EQUAL(-EAGAIN, metrics_report_periodic(buf, sizeof(buf),
			mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	/* Cycle 4 at t=4600 (=1000+3600): interval elapsed from t=1000.
	 * If last_report_time had advanced to t=2000 or t=3000 during
	 * drain, 4600-3000=1600 < 3600 and no snapshot would occur.
	 * The snapshot triggering here proves drain did not advance it. */
	mock().expectOneCall("get_timestamp").andReturnValue(4600UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	/* snapshot: prepare → collect → mfs_write → reset */
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("mfs_write")
		.withParameter("fs", mfs)
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.andReturnValue(0);
	mock().expectOneCall("reset");
	/* then report_once drains backlog */
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
	LONGS_EQUAL(-EAGAIN, metrics_report_periodic(buf, sizeof(buf),
			mfs, NULL));
}

TEST(MetricsReporterPeriodic, ShouldSnapshotAfterFrequentDrainCalls_WhenIntervalFinallyElapses) {
	/* Cycle 1 at t=1000: transmit fails → persisted */
	mock().expectOneCall("get_timestamp").andReturnValue(1000UL);
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
	LONGS_EQUAL(-EAGAIN, metrics_report_periodic(buf, sizeof(buf),
			mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	/* Cycle 2 at t=2800: drain, transmit fails → backlog grows */
	mock().expectOneCall("get_timestamp").andReturnValue(2800UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
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
	LONGS_EQUAL(-EAGAIN, metrics_report_periodic(buf, sizeof(buf),
			mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	/* Cycle 3 at t=4600 (=1000+3600): interval elapsed, backlog exists.
	 * Despite drain calls at t=2800, interval is measured from t=1000
	 * → snapshot triggers. */
	mock().expectOneCall("get_timestamp").andReturnValue(4600UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	/* snapshot: prepare → collect → mfs_write → reset */
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("mfs_write")
		.withParameter("fs", mfs)
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.andReturnValue(0);
	mock().expectOneCall("reset");
	/* then report_once drains backlog */
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
	LONGS_EQUAL(-EAGAIN, metrics_report_periodic(buf, sizeof(buf),
			mfs, NULL));
}

TEST(MetricsReporterPeriodic, ShouldReturnSnapshotError_WhenBufferTooSmall) {
	/* Cycle 1 at t=1000: normal success */
	mock().expectOneCall("get_timestamp").andReturnValue(1000UL);
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	/* Cycle 2 at t=4600: interval elapsed, backlog exists,
	 * but collect returns more than bufsize → -ENOBUFS */
	mock().expectOneCall("get_timestamp").andReturnValue(4600UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)sizeof(buf) + 1);
	LONGS_EQUAL(-ENOBUFS, metrics_report_periodic(buf, sizeof(buf),
			mfs, NULL));
}

TEST(MetricsReporterPeriodic, ShouldNotSnapshot_WhenIntervalElapsedButNoBacklog) {
	/* First call at t=1000 */
	mock().expectOneCall("get_timestamp").andReturnValue(1000UL);
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	/* Interval elapsed, no backlog → normal report_once, no snapshot */
	mock().expectOneCall("get_timestamp")
		.andReturnValue(1000UL + 3600UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
	/* no snapshot calls — goes straight to report_once */
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporterPeriodic, ShouldNotSnapshot_WhenBacklogExistsButIntervalNotElapsed) {
	/* First call at t=1000 succeeds */
	mock().expectOneCall("get_timestamp").andReturnValue(1000UL);
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	/* Backlog exists but interval NOT elapsed → drain without snapshot */
	mock().expectOneCall("get_timestamp").andReturnValue(1000UL + 10UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	/* no snapshot — goes straight to report_once */
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporterPeriodic, ShouldAlwaysReport_WhenTimestampReturnsZero) {
	mock().expectOneCall("get_timestamp").andReturnValue(0UL);
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	mock().expectOneCall("get_timestamp").andReturnValue(0UL);
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
}


TEST(MetricsReporterPeriodic, ShouldReport_WhenIntervalElapsed) {
	mock().expectOneCall("get_timestamp").andReturnValue(1000UL);
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	mock().expectOneCall("get_timestamp")
		.andReturnValue(1000UL + 3600UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(0);
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
}

/* ---- Scenario: snapshot write failure must not advance last_report_time ---- */
TEST(MetricsReporterPeriodic, ShouldRetrySnapshot_WhenPreviousSnapshotWriteFailed) {
	/* Cycle 1 at t=1000: normal success, last_report_time = 1000 */
	mock().expectOneCall("get_timestamp").andReturnValue(1000UL);
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
	LONGS_EQUAL(-EAGAIN, metrics_report_periodic(buf, sizeof(buf),
			mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	/* Cycle 2 at t=4600: interval elapsed (4600-1000=3600), backlog exists.
	 * Snapshot: metricfs_write fails (-ENOSPC).
	 * Without this fix, the snapshot error would be swallowed and
	 * last_report_time would advance to 4600 anyway.
	 * This test verifies that -ENOSPC is propagated so last_report_time
	 * stays at 1000. */
	mock().expectOneCall("get_timestamp").andReturnValue(4600UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("mfs_write")
		.withParameter("fs", mfs)
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.andReturnValue(-ENOSPC);
	/* snapshot failed → expect error propagation, no drain attempt */
	LONGS_EQUAL(-ENOSPC, metrics_report_periodic(buf, sizeof(buf),
			mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	/* Cycle 3 at t=7000: 7000-1000=6000 >= 3600 → interval elapsed.
	 * If last_report_time was incorrectly advanced to 4600,
	 * 7000-4600=2400 < 3600 → no snapshot, BUG proven.
	 * Correct behavior: last_report_time=1000, snapshot retries. */
	mock().expectOneCall("get_timestamp").andReturnValue(7000UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	/* snapshot retry: prepare → collect → mfs_write (succeeds this time) → reset */
	mock().expectOneCall("prepare").withParameter("ctx", (void *)NULL);
	mock().expectOneCall("collect")
		.withParameter("buf", buf)
		.withParameter("bufsize", (int)sizeof(buf))
		.andReturnValue((int)FAKE_PAYLOAD_LEN);
	mock().expectOneCall("mfs_write")
		.withParameter("fs", mfs)
		.withParameter("data", buf)
		.withParameter("datasize", (int)FAKE_PAYLOAD_LEN)
		.andReturnValue(0);
	mock().expectOneCall("reset");
	/* then report_once drains backlog */
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
	LONGS_EQUAL(-EAGAIN, metrics_report_periodic(buf, sizeof(buf),
			mfs, NULL));
}

/* ---- Scenario: first-call failure with mfs==NULL suppresses retries ---- */
TEST(MetricsReporterPeriodic, ShouldNotSuppressRetry_WhenFirstCallFailsWithoutMfs) {
	/* Cycle 1 at t=1000: transmit fails, mfs==NULL → no persistence.
	 * BUG: last_report_time=1000 and periodic_initialized=true set
	 * unconditionally, so next call within interval returns -EALREADY. */
	mock().expectOneCall("get_timestamp").andReturnValue(1000UL);
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
	int err = metrics_report_periodic(buf, sizeof(buf), NULL, NULL);
	LONGS_EQUAL(-EIO, err);
	mock().checkExpectations();
	mock().clear();

	/* Cycle 2 at t=1010: within interval, no backlog (mfs==NULL).
	 * BUG: returns -EALREADY, suppressing retry of unsent metrics.
	 * FIX: first call that failed should not initialize the timer. */
	mock().expectOneCall("get_timestamp").andReturnValue(1010UL);
	/* Should NOT return -EALREADY. Should attempt report again. */
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), NULL, NULL));
}

/* ---- Scenario: clock rollback causes spurious snapshot ---- */
TEST(MetricsReporterPeriodic, ShouldNotSnapshotSpuriously_WhenClockRollsBack) {
	/* Cycle 1 at t=10000: normal success */
	mock().expectOneCall("get_timestamp").andReturnValue(10000UL);
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	/* Cycle 2 at t=5000: clock rolled back (NTP correction).
	 * now(5000) - last_report_time(10000) wraps to huge uint64 value
	 * → interval_elapsed=true spuriously.
	 * With backlog: triggers unwanted snapshot.
	 * FIX: detect rollback, reset baseline, no snapshot. */
	mock().expectOneCall("get_timestamp").andReturnValue(5000UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
	/* Should NOT trigger snapshot. Should just drain backlog normally. */
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
}

TEST(MetricsReporterPeriodic, ShouldReportImmediately_WhenBacklogExists) {
	mock().expectOneCall("get_timestamp").andReturnValue(1000UL);
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
	mock().checkExpectations();
	mock().clear();

	mock().expectOneCall("get_timestamp").andReturnValue(1000UL + 10UL);
	mock().expectOneCall("mfs_count")
		.withParameter("fs", mfs).andReturnValue(1);
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
	LONGS_EQUAL(0, metrics_report_periodic(buf, sizeof(buf), mfs, NULL));
}
