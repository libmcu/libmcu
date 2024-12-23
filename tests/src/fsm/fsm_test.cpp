/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/fsm.h"

enum {
	A,
	B,
	C,
};

static bool is_state_a(fsm_state_t state, fsm_state_t next_state, void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static bool is_state_b(fsm_state_t state, fsm_state_t next_state, void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static bool is_state_c(fsm_state_t state, fsm_state_t next_state, void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static void do_state_a(fsm_state_t state, fsm_state_t next_state, void *ctx) {
	mock().actualCall(__func__);
}

static void do_state_b(fsm_state_t state, fsm_state_t next_state, void *ctx) {
	mock().actualCall(__func__);
}

static void do_state_c(fsm_state_t state, fsm_state_t next_state, void *ctx) {
	mock().actualCall(__func__);
}

static void on_state_change(struct fsm *fsm, fsm_state_t new_state,
		fsm_state_t prev_state, void *ctx) {
	mock().actualCall(__func__).withParameter("new_state", new_state)
		.withParameter("prev_state", prev_state);
}

static const struct fsm_item transitions[] = {
	FSM_ITEM(A, NULL,       do_state_a, A),
	FSM_ITEM(A, is_state_b, do_state_b, B),
	FSM_ITEM(A, is_state_c, do_state_c, C),
	FSM_ITEM(B, is_state_a, do_state_a, A),
	FSM_ITEM(B, is_state_c, do_state_c, C),
	FSM_ITEM(C, is_state_a, do_state_a, A),
	FSM_ITEM(C, is_state_b, do_state_b, B),
	FSM_ITEM(C, is_state_c, NULL,       C),
};

TEST_GROUP(FSM) {
	struct fsm fsm;

	void setup(void) {
		fsm_init(&fsm, transitions,
				sizeof(transitions) / sizeof(*transitions), 0);
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(FSM, ShouldBeStateA_WhenInitialized) {
	LONGS_EQUAL(A, fsm_state(&fsm));
}

TEST(FSM, ShouldCheckFirstRegisteredEventFirst) {
	mock().expectOneCall("is_state_b").andReturnValue(false);
	mock().ignoreOtherCalls();
	fsm_step(&fsm);
}

TEST(FSM, ShouldCheckSecondRegisteredEvent_WhenFirstEventReturnFalse) {
	mock().expectOneCall("is_state_b").andReturnValue(false);
	mock().expectOneCall("is_state_c").andReturnValue(false);
	mock().ignoreOtherCalls();
	fsm_step(&fsm);
}

TEST(FSM, ShouldCheckAllEventsInThestate_WhenNoEventReturnTrue) {
	mock().expectOneCall("is_state_b").andReturnValue(false);
	mock().expectOneCall("is_state_c").andReturnValue(false);
	fsm_step(&fsm);
}

TEST(FSM, ShouldCallAction_WhenEventReturnTrue) {
	mock().expectOneCall("is_state_b").andReturnValue(true);
	mock().expectOneCall("do_state_b");
	fsm_step(&fsm);
}

TEST(FSM, ShouldChangeState_WhenEventReturnTrue) {
	mock().expectOneCall("is_state_b").andReturnValue(true);
	mock().expectOneCall("do_state_b");
	LONGS_EQUAL(B, fsm_step(&fsm));
	LONGS_EQUAL(B, fsm_state(&fsm));
}

TEST(FSM, ShouldIgnoreNullAction_WhenEventReturnTrue) {
	mock().expectOneCall("is_state_b").andReturnValue(false);
	mock().expectOneCall("is_state_c").andReturnValue(true);
	mock().expectOneCall("do_state_c");
	fsm_step(&fsm);
	mock().expectOneCall("is_state_a").andReturnValue(false);
	mock().expectOneCall("is_state_b").andReturnValue(false);
	mock().expectOneCall("is_state_c").andReturnValue(true);
	fsm_step(&fsm);
}

TEST(FSM, ShouldCallCallback_WhenStateChange) {
	fsm_set_state_change_cb(&fsm, on_state_change, 0);
	mock().expectOneCall("on_state_change").withParameter("new_state", B)
		.withParameter("prev_state", A);
	mock().expectOneCall("is_state_b").andReturnValue(true);
	mock().expectOneCall("do_state_b");
	fsm_step(&fsm);
}

TEST(FSM, ShouldNotCallCallback_WhenStateIsNotChanged) {
	fsm_set_state_change_cb(&fsm, on_state_change, 0);
	mock().expectOneCall("is_state_b").andReturnValue(false);
	mock().expectOneCall("is_state_c").andReturnValue(false);
	fsm_step(&fsm);
}

TEST(FSM, ShouldIgnoreNullEvent) {
	mock().expectOneCall("is_state_b").andReturnValue(false);
	mock().expectOneCall("is_state_c").andReturnValue(false);
	fsm_step(&fsm);
}
