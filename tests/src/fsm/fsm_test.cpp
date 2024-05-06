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
	READY,
	OCCUPIED,
	CHARGING,
	UNAVAILABLE,
};

static bool is_plugged_in(fsm_state_t state, fsm_state_t next_state,
		void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static bool is_plugged_out(fsm_state_t state, fsm_state_t next_state,
		void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static bool has_rfid_tagged(fsm_state_t state, fsm_state_t next_state,
		void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static bool has_remotely_started(fsm_state_t state, fsm_state_t next_state,
		void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static bool has_remotely_stopped(fsm_state_t state, fsm_state_t next_state,
		void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static bool is_timed_out(fsm_state_t state, fsm_state_t next_state, void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static bool is_suspended_by_ev(fsm_state_t state, fsm_state_t next_state,
		void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static bool is_resumed_from_suspended(fsm_state_t state, fsm_state_t next_state,
		void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static bool has_hardware_error(fsm_state_t state, fsm_state_t next_state,
		void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static bool is_hardware_recovered(fsm_state_t state, fsm_state_t next_state,
		void *ctx) {
	return mock().actualCall(__func__).returnBoolValueOrDefault(false);
}

static void turn_relay_on(fsm_state_t state, fsm_state_t next_state,
		void *ctx) {
	mock().actualCall(__func__);
}

static void turn_relay_off(fsm_state_t state, fsm_state_t next_state,
		void *ctx) {
	mock().actualCall(__func__);
}

TEST_GROUP(FSM) {
	struct fsm fsm;

	void setup(void) {
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(FSM, test) {
	struct fsm_item items[] = {
		FSM_ITEM(READY, is_plugged_in, NULL, OCCUPIED),
		FSM_ITEM(READY, has_rfid_tagged, NULL, OCCUPIED),
		FSM_ITEM(READY, has_remotely_started, NULL, OCCUPIED),
		FSM_ITEM(OCCUPIED, is_plugged_in, turn_relay_on, CHARGING),
		FSM_ITEM(OCCUPIED, has_rfid_tagged, turn_relay_on, CHARGING),
		FSM_ITEM(OCCUPIED, has_remotely_started, turn_relay_on, CHARGING),
		FSM_ITEM(OCCUPIED, is_timed_out, NULL, READY),
		FSM_ITEM(OCCUPIED, is_plugged_out, NULL, READY),
		FSM_ITEM(CHARGING, is_plugged_out, turn_relay_off, READY),
		FSM_ITEM(CHARGING, has_rfid_tagged, turn_relay_off, OCCUPIED),
		FSM_ITEM(CHARGING, has_remotely_stopped, turn_relay_off, OCCUPIED),
		FSM_ITEM(CHARGING, is_suspended_by_ev, turn_relay_off, CHARGING),
		FSM_ITEM(CHARGING, is_resumed_from_suspended, turn_relay_on, CHARGING),
		FSM_ITEM(READY, has_hardware_error, NULL, UNAVAILABLE),
		FSM_ITEM(OCCUPIED, has_hardware_error, NULL, UNAVAILABLE),
		FSM_ITEM(CHARGING, has_hardware_error, NULL, UNAVAILABLE),
		FSM_ITEM(UNAVAILABLE, is_hardware_recovered, NULL, READY),
	};

	fsm_init(&fsm, items, sizeof(items) / sizeof(*items), 0);

	mock().expectOneCall("is_plugged_in").andReturnValue(true);
	fsm_step(&fsm);
	mock().expectOneCall("is_plugged_in").andReturnValue(true);
	mock().expectOneCall("turn_relay_on");
	fsm_step(&fsm);
	mock().expectOneCall("is_plugged_out").andReturnValue(false);
	mock().expectOneCall("has_rfid_tagged").andReturnValue(false);
	mock().expectOneCall("has_remotely_stopped").andReturnValue(false);
	mock().expectOneCall("is_suspended_by_ev").andReturnValue(true);
	mock().expectOneCall("turn_relay_off");
	fsm_step(&fsm);
	mock().expectOneCall("is_plugged_out").andReturnValue(true);
	mock().expectOneCall("turn_relay_off");
	fsm_step(&fsm);
}
