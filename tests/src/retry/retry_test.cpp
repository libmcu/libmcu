/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/retry.h"
#include "libmcu/retry_overrides.h"

void retry_sleep_ms(unsigned int msec)
{
	mock().actualCall(__func__).withParameter("msec", msec);
}

int retry_generate_random(void)
{
	return mock().actualCall(__func__).returnIntValueOrDefault(0);
}

TEST_GROUP(retry) {
	struct retry retry;

	void setup(void) {
		retry_init(&retry, 10, 2560000, 5000, 5000);
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(retry, next_ShouldBackoffExponentially) {
	uint32_t expected = 5000;
	mock().ignoreOtherCalls();
	for (int i = 0; i < 10; i++) {
		LONGS_EQUAL(expected, retry_backoff_next(&retry));
		expected *= 2;
	}
}

TEST(retry, next_ShouldBackoffExponentiallyWithJitter) {
	uint32_t expected = 1234;
	retry_init(&retry, 10, 2560000, 0, 5000);
	mock().expectOneCall("retry_generate_random").andReturnValue((int)expected);
	LONGS_EQUAL(expected, retry_backoff_next(&retry));
}

TEST(retry, next_ShouldWrapAroundJitter_WhenGreaterThanMaxJitterGiven) {
	uint32_t expected = 234;
	retry_init(&retry, 10, 2560000, 0, 1000);
	mock().expectOneCall("retry_generate_random").andReturnValue((int)expected);
	LONGS_EQUAL(expected, retry_backoff_next(&retry));
}

TEST(retry, retry_ShouldRetry3Times_When3GivenForMaxAttempts) {
	retry_init(&retry, 3, 0, 0, 0);
	int cnt = 0;
	while (retry_backoff(&retry) != RETRY_EXHAUSTED) { cnt++; }
	LONGS_EQUAL(3, cnt);
}

TEST(retry, retry_ShouldRetry1Time_When1GivenForMaxAttempts) {
	retry_init(&retry, 1, 0, 0, 0);
	int cnt = 0;
	while (retry_backoff(&retry) != RETRY_EXHAUSTED) { cnt++; }
	LONGS_EQUAL(1, cnt);
}

TEST(retry, retry_ShouldRetry0Time_When0GivenForMaxAttempts) {
	retry_init(&retry, 0, 0, 0, 0);
	int cnt = 0;
	while (retry_backoff(&retry) != RETRY_EXHAUSTED) { cnt++; }
	LONGS_EQUAL(0, cnt);
}

TEST(retry, retry_ShouldSleepForInRange) {
	uint32_t base = retry.min_backoff_ms;
	mock().expectNCalls(10, "retry_generate_random");
	mock().expectNCalls(10, "retry_sleep_ms").ignoreOtherParameters();
	while (retry_backoff(&retry) != RETRY_EXHAUSTED) {
		CHECK(retry.previous_backoff_ms <= base + retry.max_jitter_ms);
		base = retry.previous_backoff_ms * 2;
	}
}

TEST(retry, init_ShouldSetMaxJitterMsToMinBackOffMs_WhenMinBackoffGreater) {
	retry_init(&retry, 10, 1, 2, 0);
	LONGS_EQUAL(2, retry.max_backoff_ms);
}

TEST(retry, reset_ShouldResetAttemptsAndPreviousBackOffMs) {
	mock().expectOneCall("retry_generate_random");
	retry_backoff_next(&retry);
	LONGS_EQUAL(1, retry.attempts);
	retry_reset(&retry);
	LONGS_EQUAL(0, retry.attempts);
}

TEST(retry, exhausted_ShouldReturnTrue_WhenExhausted) {
	retry_init(&retry, 0, 0, 0, 0);
	LONGS_EQUAL(1, retry_exhausted(&retry));

	retry_init(&retry, 1, 0, 0, 0);
	retry_backoff_next(&retry);
	LONGS_EQUAL(1, retry_exhausted(&retry));
}

TEST(retry, exhausted_ShouldReturnFalse_WhenNotExhausted) {
	mock().ignoreOtherCalls();
	for (uint16_t i = 0; i < retry.max_attempts; i++) {
		LONGS_EQUAL(0, retry_exhausted(&retry));
		retry_backoff_next(&retry);
	}
	LONGS_EQUAL(1, retry_exhausted(&retry));
}
