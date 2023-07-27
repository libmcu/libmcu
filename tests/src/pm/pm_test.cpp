/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/pm.h"
#include "libmcu/port/pm.h"

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

int pm_port_enter(pm_mode_t mode, uint32_t duration_ms)
{
	(void)mode;
	(void)duration_ms;
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

TEST(PM, register_ShouldSortEntriesInOrderOfPriority_WhenMixedPrioirtiesGiven) {
	int t1, t2, t3;
	mock().expectOneCall("cb_1").withParameter("ctx", &t1);
	mock().expectOneCall("cb_2").withParameter("ctx", &t2);
	mock().expectOneCall("cb_3").withParameter("ctx", &t3);
	pm_register_entry_callback(PM_SLEEP, 0, cb_1, &t1);
	pm_register_entry_callback(PM_SLEEP, 2, cb_2, &t2);
	pm_register_entry_callback(PM_SLEEP, 1, cb_3, &t3);

	pm_enter(PM_SLEEP, 0);

	LONGS_EQUAL(0, t2);
	LONGS_EQUAL(1, t3);
	LONGS_EQUAL(2, t1);
}

TEST(PM, unregister_ShouldRemoveEntry_WhenExists) {
	int t1, t2, t3;
	mock().expectOneCall("cb_1").withParameter("ctx", &t1);
	mock().expectOneCall("cb_3").withParameter("ctx", &t3);
	pm_register_entry_callback(PM_SLEEP, 0, cb_1, &t1);
	pm_register_entry_callback(PM_SLEEP, 1, cb_2, &t2);
	pm_register_entry_callback(PM_SLEEP, 2, cb_3, &t3);
	pm_unregister_entry_callback(PM_SLEEP, 1, cb_2);

	pm_enter(PM_SLEEP, 0);
}

TEST(PM, register_ShouldReturnEINVAL_WhenNullFuncGiven) {
	LONGS_EQUAL(-EINVAL, pm_register_entry_callback(PM_SLEEP, 0, 0, 0));
}

TEST(PM, unregister_ShouldReturnNOENT_WhenNoEntryFound) {
	LONGS_EQUAL(-ENOENT, pm_unregister_entry_callback(PM_SLEEP, 0, cb_1));
}

TEST(PM, unregister_ShouldReturnNOENT_WhenNullEntryGiven) {
	LONGS_EQUAL(-ENOENT, pm_unregister_entry_callback(PM_SLEEP, 0, 0));
}

TEST(PM, register_ShouldReturnNOSPC_WhenFull) {
	pm_callback_t cb[PM_CALLBACK_MAXLEN];
	pm_callback_t cb_extra = (pm_callback_t)(PM_CALLBACK_MAXLEN + 1);

	for (unsigned int i = 0; i < PM_CALLBACK_MAXLEN; i++) {
		cb[i] = (pm_callback_t)(uintptr_t)(i+1);
		LONGS_EQUAL(0, pm_register_entry_callback(PM_SLEEP, 0, cb[i], 0));
	}

	LONGS_EQUAL(-ENOSPC, pm_register_entry_callback(PM_SLEEP, 0, cb_extra, 0));
}

TEST(PM, register_ShouldReturnEXIST_WhenDuplicated) {
	LONGS_EQUAL(0, pm_register_entry_callback(PM_SLEEP, 0, cb_1, 0));
	LONGS_EQUAL(-EEXIST, pm_register_entry_callback(PM_SLEEP, 0, cb_1, 0));
}
