/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/pm.h"
#include "libmcu/board/pm.h"

static int cnt;

static void cb_1(void *ctx) {
	int *t = (int *)ctx;
	*t = cnt++;
	mock().actualCall(__func__).withParameter("ctx", ctx);
}

static void cb_2(void *ctx) {
	int *t = (int *)ctx;
	*t = cnt++;
	mock().actualCall(__func__).withParameter("ctx", ctx);
}

static void cb_3(void *ctx) {
	int *t = (int *)ctx;
	*t = cnt++;
	mock().actualCall(__func__).withParameter("ctx", ctx);
}

int pm_board_enter(pm_mode_t mode)
{
	(void)mode;
	return 0;
}

TEST_GROUP(PM) {
	void setup(void) {
		pm_init();
		cnt = 0;
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(PM, register_callback_ShouldSortEntriesInOrderOfPriority_WhenMixPrioiryGiven) {
	int t1, t2, t3;
	mock().expectOneCall("cb_1").withParameter("ctx", &t1);
	mock().expectOneCall("cb_2").withParameter("ctx", &t2);
	mock().expectOneCall("cb_3").withParameter("ctx", &t3);
	pm_register_entry_callback(PM_SLEEP, 0, cb_1, &t1);
	pm_register_entry_callback(PM_SLEEP, 2, cb_2, &t2);
	pm_register_entry_callback(PM_SLEEP, 1, cb_3, &t3);

	pm_enter(PM_SLEEP);

	LONGS_EQUAL(0, t2);
	LONGS_EQUAL(1, t3);
	LONGS_EQUAL(2, t1);
}
