/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/trace.h"

static void callback(const struct trace *entry, void *ctx) {
	mock().actualCall(__func__)
		.withParameterOfType("traceType", "entry", entry)
		.withParameter("ctx", ctx);
}

class TraceComparator : public MockNamedValueComparator
{
public:
	virtual bool isEqual(const void *object1, const void *object2)
	{
		if (!object1 || !object2 ||
				memcmp(object1, object2, sizeof(struct trace))) {
			return false;
		}

		return true;
	}
	virtual SimpleString valueToString(const void *object)
	{
		return StringFrom(object);
	}
};

TEST_GROUP(trace) {
	void setup(void) {
		static TraceComparator traceComparator;
		mock().installComparator("traceType", traceComparator);

		trace_reset();
	}
	void teardown() {
		mock().checkExpectations();
		mock().removeAllComparatorsAndCopiers();
		mock().clear();
	}
};

TEST(trace, iterate_ShouldRunCallback_WhenAnEntryExists) {
	struct trace expected = {
		.callee = (void *)1234,
		.depth = 0,
	};

	mock().expectOneCall("callback")
		.withParameterOfType("traceType", "entry", &expected)
		.withParameter("ctx", (void *)NULL);

	__cyg_profile_func_enter((void *)1234, 0);
	trace_iterate(callback, NULL, -1);
}

TEST(trace, count_ShouldReturnTheNumberOfEntries) {
	for (int i = 0; i < TRACE_MAXLEN; i++) {
		__cyg_profile_func_enter(0, 0);
		LONGS_EQUAL(i+1, trace_count());
	}
}

TEST(trace, ShouldResetTraceData_WhenResetGiven) {
	__cyg_profile_func_enter(0, 0);
	trace_reset();
	LONGS_EQUAL(0, trace_count());
}

TEST(trace, ShouldClearTraceData_WhenClearGiven) {
	__cyg_profile_func_enter(0, 0);
	trace_clear();
	LONGS_EQUAL(0, trace_count());
}

TEST(trace, ShouldNotRunCallback_WhenNoEntriesExists) {
	trace_iterate(callback, NULL, -1);
}

TEST(trace, ShouldOverwrite_WhenMoreThanMaxEntriesGiven) {
	struct trace expected = {
		.callee = (void *)0xfeed,
		.depth = TRACE_MAXLEN,
	};

	for (int i = 0; i < TRACE_MAXLEN; i++) {
		__cyg_profile_func_enter(0, 0);
		LONGS_EQUAL(i+1, trace_count());
	}

	__cyg_profile_func_enter((void *)0xfeed, 0);
	LONGS_EQUAL(TRACE_MAXLEN, trace_count());

	mock().expectOneCall("callback")
		.withParameterOfType("traceType", "entry", &expected)
		.withParameter("ctx", (void *)NULL);
	trace_iterate(callback, NULL, 1);
}

TEST(trace, ShouldIncreaseDepth_WhenEnteringFunction) {
	struct trace expected0 = {
		.callee = (void *)0xfeed0,
		.depth = 0,
	};
	struct trace expected1 = {
		.callee = (void *)0xfeed1,
		.depth = 1,
	};

	__cyg_profile_func_enter((void *)0xfeed0, 0);
	__cyg_profile_func_enter((void *)0xfeed1, 0);

	mock().expectOneCall("callback")
		.withParameterOfType("traceType", "entry", &expected0)
		.withParameter("ctx", (void *)NULL);
	mock().expectOneCall("callback")
		.withParameterOfType("traceType", "entry", &expected1)
		.withParameter("ctx", (void *)NULL);
	trace_iterate(callback, NULL, -1);
}

TEST(trace, ShouldDecreaseDepth_WhenLeavingFunction) {
	struct trace expected0 = {
		.callee = (void *)0xfeed0,
		.depth = 0,
	};
	struct trace expected1 = {
		.callee = (void *)0xfeed1,
		.depth = 0,
	};

	__cyg_profile_func_enter((void *)0xfeed0, 0);
	__cyg_profile_func_exit(0, 0);
	__cyg_profile_func_enter((void *)0xfeed1, 0);

	mock().expectOneCall("callback")
		.withParameterOfType("traceType", "entry", &expected0)
		.withParameter("ctx", (void *)NULL);
	mock().expectOneCall("callback")
		.withParameterOfType("traceType", "entry", &expected1)
		.withParameter("ctx", (void *)NULL);
	trace_iterate(callback, NULL, -1);
}
