/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/ratelim.h"
#include <time.h>

#define DEFAULT_TIME		1734083880
#define DEFAULT_CAP		10
#define DEFAULT_RATE		1

time_t time(time_t *t) {
	return (time_t)mock().actualCall(__func__).returnIntValueOrDefault(0);
}

static void format_func(const char *format, va_list args) {
	mock().actualCall(__func__).withStringParameter("format", format);
}

TEST_GROUP(RateLim) {
	struct ratelim bucket;

	void setup(void) {
		mock().expectOneCall("time").andReturnValue(DEFAULT_TIME);
		ratelim_init(&bucket,
				RATELIM_UNIT_SECOND, DEFAULT_CAP, DEFAULT_RATE);
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(RateLim, full_ShouldReturnFalse_WhenBucketIsEmpty) {
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME);
	CHECK_FALSE(ratelim_full(&bucket));
}

TEST(RateLim, full_ShouldReturnFalse_WhenBucketIsNotFull) {
	mock().expectNCalls(DEFAULT_CAP, "time").andReturnValue(DEFAULT_TIME);
	for (int i = 0; i < DEFAULT_CAP-1; i++) {
		ratelim_request(&bucket);
	}
	CHECK_FALSE(ratelim_full(&bucket));
}

TEST(RateLim, full_ShouldReturnTrue_WhenBucketIsFull) {
	mock().expectNCalls(DEFAULT_CAP+1, "time").andReturnValue(DEFAULT_TIME);
	for (int i = 0; i < DEFAULT_CAP; i++) {
		ratelim_request(&bucket);
	}
	CHECK_TRUE(ratelim_full(&bucket));
}

TEST(RateLim, request_ShouldReturnTrue_WhenBucketIsNotFull) {
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME);
	CHECK_TRUE(ratelim_request(&bucket));
}

TEST(RateLim, request_ShouldReturnFalse_WhenBucketIsFull) {
	mock().expectNCalls(DEFAULT_CAP+1, "time").andReturnValue(DEFAULT_TIME);
	for (int i = 0; i < DEFAULT_CAP; i++) {
		ratelim_request(&bucket);
	}
	CHECK_FALSE(ratelim_request(&bucket));
}

TEST(RateLim, request_ShouldReturnTrue_WhenBucketIsNotFullAfterLeak) {
	mock().expectNCalls(DEFAULT_CAP+1, "time").andReturnValue(DEFAULT_TIME);
	for (int i = 0; i < DEFAULT_CAP; i++) {
		ratelim_request(&bucket);
	}
	CHECK_FALSE(ratelim_request(&bucket));
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+1);
	CHECK_TRUE(ratelim_request(&bucket));
}

TEST(RateLim, request_ShouldReturnFalse_WhenBucketIsFullAfterLeak) {
	mock().expectNCalls(DEFAULT_CAP+1, "time").andReturnValue(DEFAULT_TIME);
	for (int i = 0; i < DEFAULT_CAP; i++) {
		ratelim_request(&bucket);
	}
	CHECK_FALSE(ratelim_request(&bucket));
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+1);
	CHECK_TRUE(ratelim_request(&bucket));
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+1);
	CHECK_FALSE(ratelim_request(&bucket));
}

TEST(RateLim, request_ShouldReturnTrue_WhenBucketIsNotFullAfterMultipleLeak) {
	mock().expectNCalls(DEFAULT_CAP+1, "time").andReturnValue(DEFAULT_TIME);
	for (int i = 0; i < DEFAULT_CAP; i++) {
		ratelim_request(&bucket);
	}
	CHECK_FALSE(ratelim_request(&bucket));
	for (int i = 0; i < DEFAULT_CAP; i++) {
		mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+DEFAULT_CAP);
		CHECK_TRUE(ratelim_request(&bucket));
	}
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+DEFAULT_CAP);
	CHECK_FALSE(ratelim_request(&bucket));
}

TEST(RateLim, request_ShouldReturnTrue_WhenBucketLeakHappenEverySecondAndRequestEverySecond) {
	for (int i = 0; i < DEFAULT_CAP*10; i++) {
		mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+i);
		CHECK_TRUE(ratelim_request(&bucket));
	}
}

TEST(RateLim, request_ShouldReturnFalse_WhenCapacityIsZero) {
	mock().expectNCalls(2, "time").andReturnValue(DEFAULT_TIME);
	ratelim_init(&bucket, RATELIM_UNIT_SECOND, 0, DEFAULT_RATE);
	CHECK_FALSE(ratelim_request(&bucket));
}

TEST(RateLim, request_ShouldReturnFalse_WhenBucketIsFullAndLeakRateIsZero) {
	mock().expectNCalls(DEFAULT_CAP+1, "time").andReturnValue(DEFAULT_TIME);
	ratelim_init(&bucket, RATELIM_UNIT_SECOND, DEFAULT_CAP, 0);
	for (int i = 0; i < DEFAULT_CAP; i++) {
		CHECK_TRUE(ratelim_request(&bucket));
	}
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+DEFAULT_CAP*2);
	CHECK_FALSE(ratelim_request(&bucket));
}

TEST(RateLim, request_format_ShouldCallFormatFunc_WhenRequestIsSuccessful) {
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME);
	mock().expectOneCall("format_func")
		.withStringParameter("format", "hello, world")
		.andReturnValue(0);
	ratelim_request_format(&bucket, format_func, "hello, world");
}

TEST(RateLim, ShouldCalculatePerHourBase_WhenUnitIsHour) {
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME);
	ratelim_init(&bucket, RATELIM_UNIT_HOUR, DEFAULT_CAP, DEFAULT_RATE);
	for (int i = 0; i < DEFAULT_CAP; i++) {
		mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+i);
		CHECK_TRUE(ratelim_request(&bucket));
	}
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+DEFAULT_CAP);
	CHECK_FALSE(ratelim_request(&bucket));

	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+3600);
	CHECK_TRUE(ratelim_request(&bucket));
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+3600);
	CHECK_FALSE(ratelim_request(&bucket));
}

TEST(RateLim, ShouldCalculatePerMinuteBase_WhenUnitIsMinute) {
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME);
	ratelim_init(&bucket, RATELIM_UNIT_MINUTE, DEFAULT_CAP, DEFAULT_RATE);
	for (int i = 0; i < DEFAULT_CAP; i++) {
		mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+i);
		CHECK_TRUE(ratelim_request(&bucket));
	}
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+DEFAULT_CAP);
	CHECK_FALSE(ratelim_request(&bucket));

	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+60);
	CHECK_TRUE(ratelim_request(&bucket));
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+60);
	CHECK_FALSE(ratelim_request(&bucket));
}

TEST(RateLim, ShouldLeakMultipleTokens_WhenRequestMultipleTokens) {
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME);
	ratelim_init(&bucket, RATELIM_UNIT_SECOND, DEFAULT_CAP, DEFAULT_RATE);
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME);
	CHECK_TRUE(ratelim_request_ext(&bucket, DEFAULT_CAP));
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME);
	CHECK_FALSE(ratelim_request_ext(&bucket, 1));
}

TEST(RateLim, request_ext_ShouldReturnFalse_WhenRequestMoreThanCapacity) {
	CHECK_FALSE(ratelim_request_ext(&bucket, DEFAULT_CAP+1));
}

TEST(RateLim, request_ext_ShouldReturnTrue_WhenZeroTokenRequested) {
	CHECK_TRUE(ratelim_request_ext(&bucket, 0));
}

TEST(RateLim, ShouldLeakEveryHalfMinute_WhenRateIsTwoPerMinute) {
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME);
	ratelim_init(&bucket, RATELIM_UNIT_MINUTE, DEFAULT_CAP, 2);
	for (int i = 0; i < DEFAULT_CAP; i++) {
		mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+i);
		CHECK_TRUE(ratelim_request(&bucket));
	}
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+30-1);
	CHECK_FALSE(ratelim_request(&bucket));
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+30);
	CHECK_TRUE(ratelim_request(&bucket));
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+30+30-1);
	CHECK_FALSE(ratelim_request(&bucket));
	mock().expectOneCall("time").andReturnValue(DEFAULT_TIME+30+30);
	CHECK_TRUE(ratelim_request(&bucket));
}
