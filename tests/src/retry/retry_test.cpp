/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/retry.h"

TEST_GROUP(retry) {
	struct retry retry;

	void setup(void) {
		struct retry_param param = {
			.max_attempts = 10,
			.min_backoff_ms = 5000,
			.max_backoff_ms = 2560000,
			.max_jitter_ms = 5000,
		};
		retry_new_static(&retry, &param);
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(retry, ShouldBackoffExponentially_WhenZeroJitterGiven) {
	uint32_t expected = 5000;
	uint32_t actual;
	for (int i = 0; i < 10; i++) {
		LONGS_EQUAL(RETRY_ERROR_NONE, retry_backoff(&retry, &actual, 0));
		LONGS_EQUAL(expected, actual);
		expected *= 2;
	}
}

TEST(retry, ShouldBackoffExponentially_WhenNonZeroJitterGiven) {
	uint32_t expected = 1234;
	uint32_t actual;
	LONGS_EQUAL(RETRY_ERROR_NONE, retry_backoff(&retry, &actual, (uint16_t)expected));
	LONGS_EQUAL(expected+5000, actual);
}

TEST(retry, ShouldWrapAroundJitter_WhenGreaterThanMaxJitterGiven) {
	uint32_t expected = 6234;
	uint32_t actual;
	LONGS_EQUAL(RETRY_ERROR_NONE, retry_backoff(&retry, &actual, (uint16_t)expected));
	LONGS_EQUAL(expected, actual);
}

TEST(retry, ShouldReturnExhausted_WhenMaxAttemptsReached) {
	uint32_t actual;
	for (int i = 0; i < 10; i++) {
		LONGS_EQUAL(RETRY_ERROR_NONE, retry_backoff(&retry, &actual, 0));
	}
	LONGS_EQUAL(RETRY_ERROR_EXHAUSTED, retry_backoff(&retry, &actual, 0));
}

TEST(retry, ShouldReturnExhausted_WhenMaxAttemptsReached_WhenMaxAttempts1Given) {
	uint32_t actual;
	struct retry_param param = {
		.max_attempts = 1,
	};
	retry_new_static(&retry, &param);
	LONGS_EQUAL(RETRY_ERROR_NONE, retry_backoff(&retry, &actual, 0));
	LONGS_EQUAL(RETRY_ERROR_EXHAUSTED, retry_backoff(&retry, &actual, 0));
}

TEST(retry, ShouldReturnExhausted_WhenMaxAttemptsReached_WhenMaxAttempts0Given) {
	uint32_t actual;
	struct retry_param param = {
		.max_attempts = 0,
	};
	retry_new_static(&retry, &param);
	LONGS_EQUAL(RETRY_ERROR_EXHAUSTED, retry_backoff(&retry, &actual, 0));
}

TEST(retry, ShouldReturnInvalidParam_WhenNullRetryGiven) {
	LONGS_EQUAL(RETRY_ERROR_INVALID_PARAM, retry_backoff(&retry, NULL, 0));
}

TEST(retry, ShouldReturnInvalidParam_WhenNullOutParamGiven) {
	uint32_t actual;
	LONGS_EQUAL(RETRY_ERROR_INVALID_PARAM, retry_backoff(NULL, &actual, 0));
}



TEST(retry, ShouldSetMaxJitterMsToMinBackOffMs_WhenMinBackoffGreater) {
	struct retry_param param = {
		.max_attempts = 10,
		.min_backoff_ms = 2,
		.max_backoff_ms = 1,
	};
	retry_new_static(&retry, &param);
	LONGS_EQUAL(2, retry.param.max_backoff_ms);
}

TEST(retry, ShouldResetAttemptsAndPreviousBackOffMs) {
	uint32_t actual;
	retry_backoff(&retry, &actual, 0);
	LONGS_EQUAL(1, retry.attempts);
	retry_reset(&retry);
	LONGS_EQUAL(0, retry.attempts);
}

TEST(retry, exhausted_ShouldReturnTrue_WhenExhausted) {
	uint32_t actual;
	struct retry_param param = {
		.max_attempts = 0,
	};
	retry_new_static(&retry, &param);
	LONGS_EQUAL(true, retry_exhausted(&retry));

	param.max_attempts = 1;
	retry_new_static(&retry, &param);
	LONGS_EQUAL(RETRY_ERROR_NONE, retry_backoff(&retry, &actual, 0));
	LONGS_EQUAL(true, retry_exhausted(&retry));
}

TEST(retry, exhausted_ShouldReturnFalse_WhenNotExhausted) {
	uint32_t actual;
	for (int i = 0; i < 10; i++) {
		LONGS_EQUAL(false, retry_exhausted(&retry));
		retry_backoff(&retry, &actual, 0);
	}
	LONGS_EQUAL(true, retry_exhausted(&retry));
}

TEST(retry, first_ShouldRetrunTrue_WhenCalledFirstTime) {
	LONGS_EQUAL(true, retry_first(&retry));
}

TEST(retry, first_ShouldRetrunFalse_WhenCalledSecondTime) {
	uint32_t actual;
	retry_backoff(&retry, &actual, 0);
	LONGS_EQUAL(false, retry_first(&retry));
}
