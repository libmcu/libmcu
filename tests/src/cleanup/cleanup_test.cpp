/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/cleanup.h"

TEST_GROUP(Cleanup) {
	void setup(void) {
		cleanup_init();
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();

		cleanup_deinit();
	}
};

static void test_func1(void *ctx) {
	mock().actualCall(__func__);
}

static void test_func2(void *ctx) {
	mock().actualCall(__func__);
}

TEST(Cleanup, ShouldReturnNoError_WhenValidFunctionIsRegistered) {
	LONGS_EQUAL(CLEANUP_ERROR_NONE, cleanup_register(1, test_func1, NULL));
	LONGS_EQUAL(CLEANUP_ERROR_NONE, cleanup_register(2, test_func2, NULL));
}

TEST(Cleanup, ShouldRegisteredFunctionsBeCalled_WhenCleanupIsExecuted) {
	mock().expectOneCall("test_func1");
	mock().expectOneCall("test_func2");

	cleanup_register(1, test_func1, NULL);
	cleanup_register(2, test_func2, NULL);

	cleanup_execute();
}

TEST(Cleanup, ShouldRegisteredFunctionsBeCalledInOrderOfPriority_WhenCleanupIsExecuted) {
	mock().strictOrder();
	mock().expectOneCall("test_func2");
	mock().expectOneCall("test_func1");

	cleanup_register(1, test_func1, NULL);
	cleanup_register(2, test_func2, NULL);

	cleanup_execute();
}
