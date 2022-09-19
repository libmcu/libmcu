/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include <stdlib.h>
#include <time.h>
#include "libmcu/retry.h"

static int nr_sleep_called;
void retry_sleep_ms(unsigned int msec)
{
	nr_sleep_called++;
}

int retry_generate_random(void)
{
	return rand();
}

TEST_GROUP(retry) {
	struct retry_params retry;
	void setup(void) {
		nr_sleep_called = 0;
		retry = (struct retry_params) {
			.max_backoff_ms = 300000,
			.min_backoff_ms = 5000,
			.max_jitter_ms = 5000,
			.max_attempts = 5,
		};
		srand((unsigned int)time(0));
	}
	void teardown(void) {
	}
};

TEST(retry, retry_ShouldRetry3Times_When3GivenForMaxAttempts) {
	retry.max_attempts = 3;
	while (retry_backoff(&retry) != RETRY_EXHAUSTED);
	LONGS_EQUAL(3, nr_sleep_called);
}

TEST(retry, retry_ShouldRetry1Time_When1GivenForMaxAttempts) {
	retry.max_attempts = 1;
	while (retry_backoff(&retry) != RETRY_EXHAUSTED);
	LONGS_EQUAL(1, nr_sleep_called);
}

TEST(retry, retry_ShouldRetry0Time_When0GivenForMaxAttempts) {
	retry.max_attempts = 0;
	while (retry_backoff(&retry) != RETRY_EXHAUSTED);
	LONGS_EQUAL(0, nr_sleep_called);
}

TEST(retry, retry_ShouldSleepForInRange) {
	uint32_t base = retry.min_backoff_ms;
	retry.max_attempts = 10;
	while (retry_backoff(&retry) != RETRY_EXHAUSTED) {
		CHECK(retry.previous_backoff_ms <= base + retry.max_jitter_ms);
		base = retry.previous_backoff_ms * 2;
	}
}

TEST(retry, retry_ShouldSetDefaultMaxBackOffMs_WhenZeroGivenForIt) {
	struct retry_params myretry = { 0, };
	retry_backoff(&myretry);
	LONGS_EQUAL(300000, myretry.max_backoff_ms);
}

TEST(retry, retry_ShouldSetDefaultMaxJitterMs_WhenZeroGivenForIt) {
	struct retry_params myretry = { 0, };
	retry_backoff(&myretry);
	LONGS_EQUAL(5000, myretry.max_jitter_ms);
}

TEST(retry, retry_ShouldSetMaxJitterMsToMinBackOffMs_WhenMinBackoffMsGiven) {
	struct retry_params myretry = { .min_backoff_ms = 1000, };
	retry_backoff(&myretry);
	LONGS_EQUAL(1000, myretry.max_jitter_ms);
}

TEST(retry, reset_ShouldResetAttemptsAndPreviousBackOffMs) {
	retry_backoff(&retry);
	LONGS_EQUAL(1, retry.attempts);
	CHECK(retry.previous_backoff_ms != 0);
	retry_reset(&retry);
	LONGS_EQUAL(0, retry.attempts);
	LONGS_EQUAL(0, retry.previous_backoff_ms);
}
