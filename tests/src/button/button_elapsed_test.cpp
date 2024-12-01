/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/button.h"
#include "libmcu/compiler.h"

static button_level_t get_button_state(void *ctx)
{
	return (button_level_t)
		mock().actualCall(__func__).returnIntValueOrDefault(0);
}

static void on_button_event(struct button *btn, const button_event_t event,
		const uint8_t clicks, void *ctx)
{
	mock().actualCall(__func__)
		.withParameter("event", event)
		.withParameter("clicks", clicks);
}

TEST_GROUP(ButtonElapsed) {
	struct button *button;

	void setup(void) {
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}

	void prepare(void) {
		button = button_new(get_button_state, 0, on_button_event, 0);
		CHECK(button != NULL);
		button_enable(button);
	}
	void finish(void) {
		button_disable(button);
		button_delete(button);
	}
	void step(uint32_t count) {
		for (uint32_t i = 0; i < count; i++) {
			LONGS_EQUAL(BUTTON_ERROR_NONE, button_step_elapsed(button, BUTTON_SAMPLING_INTERVAL_MS));
		}
	}
};

TEST(ButtonElapsed, new_ShouldReturnNULL_WhenNoGetStateGiven) {
	LONGS_EQUAL(0, button_new(NULL, NULL, NULL, NULL));
}

TEST(ButtonElapsed, new_ShouldReturnNull_WhenNoSlotsLeft) {
	struct button *btn[BUTTON_MAX];

	for (int i = 0; i < BUTTON_MAX; i++) {
		btn[i] = button_new(get_button_state, 0, on_button_event, 0);
		CHECK(btn[i] != NULL);
	}
	LONGS_EQUAL(0, button_new(get_button_state, 0, on_button_event, 0));

	for (int i = 0; i < BUTTON_MAX; i++) {
		button_delete(btn[i]);
	}
}

TEST(ButtonElapsed, new_ShouldReturnInstance_WhenFreeSlotAvailableAfterFull) {
	struct button *btn[BUTTON_MAX];

	for (int i = 0; i < BUTTON_MAX; i++) {
		btn[i] = button_new(get_button_state, 0, on_button_event, 0);
		CHECK(btn[i] != NULL);
	}
	LONGS_EQUAL(0, button_new(get_button_state, 0, on_button_event, 0));

	button_delete(btn[0]);
	btn[0] = button_new(get_button_state, 0, on_button_event, 0);
	CHECK(btn[0] != NULL);

	for (int i = 0; i < BUTTON_MAX; i++) {
		button_delete(btn[i]);
	}
}

TEST(ButtonElapsed, new_ShouldReturnInstance_WhenValidArgumentsGiven) {
	struct button *p = button_new(get_button_state, 0, on_button_event, 0);
	CHECK(p != NULL);
	button_delete(p);
}

TEST(ButtonElapsed, step_ShouldDoNothing_WhenNoiseGiven) {
	//0x1cf1f: 0b1_1100_1111_0001_1111
	mock().expectNCalls(3, "get_button_state").andReturnValue(1);
	mock().expectNCalls(2, "get_button_state").andReturnValue(0);
	mock().expectNCalls(4, "get_button_state").andReturnValue(1);
	mock().expectNCalls(3, "get_button_state").andReturnValue(0);
	mock().expectNCalls(5, "get_button_state").andReturnValue(1);
	mock().expectNCalls(7, "get_button_state").andReturnValue(0);

	mock().expectNoCall("on_button_event");

	prepare();
	step(3+2+4+3+5+7);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandlePressed_WhenValidPatternGiven) {
	//0x35fff: 0b11_0101_1111_1111_1111
	mock().expectNCalls(2, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);

	prepare();
	step(2+1+1+1+6);
	finish();
}

TEST(ButtonElapsed, step_ShouldIgnoreNoise_WhenNoiseGivenInMiddleOfPressed) {
	//0xffc35fff: 0b1111_1111_1100_0011_0101_1111_1111_1111
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(4, "get_button_state").andReturnValue(0);
	mock().expectNCalls(2, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(13, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);

	prepare();
	step(6+4+2+1+1+1+13);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleReleased_WhenButtonReleased) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_RELEASED)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_CLICK)
		.withParameter("clicks", 1);

	prepare();
	step(6+6);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleHolding_WhenButtonKeepPressed) {
	mock().expectNCalls(2, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.withParameter("clicks", 0);

	prepare();
	step(2+1+1+1+36);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleHoldingRepeat_WhenButtonKeepPressedLonger) {
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);
	mock().expectNCalls(20, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.withParameter("clicks", 0);

	prepare();
	step(36+20);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleHoldingRepeat_WhenButtonKeepPressedMuchLonger) {
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);
	mock().expectNCalls(200, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.withParameter("clicks", 0);
	mock().expectNCalls(10, "on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.withParameter("clicks", 0);

	prepare();
	step(36+200);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleClick_WhenTwoClickGiven) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);

	mock().expectNCalls(2, "on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);
	mock().expectNCalls(2, "on_button_event")
		.withParameter("event", BUTTON_EVT_RELEASED)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_CLICK)
		.withParameter("clicks", 1);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_CLICK)
		.withParameter("clicks", 2);

	prepare();
	step(6+6+6+6);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleClick_WhenOneClickGivenTwice) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(56, "get_button_state").andReturnValue(0);

	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(56, "get_button_state").andReturnValue(0);

	mock().expectNCalls(2, "on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);
	mock().expectNCalls(2, "on_button_event")
		.withParameter("event", BUTTON_EVT_RELEASED)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_CLICK)
		.withParameter("clicks", 1);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_CLICK)
		.withParameter("clicks", 1);

	prepare();
	step(6+56+6+56);
	finish();
}

TEST(ButtonElapsed, busy_ShouldReturnFalse_WhenInitialized) {
	prepare();
	LONGS_EQUAL(false, button_busy(button));
	finish();
}

TEST(ButtonElapsed, busy_ShouldReturnTrue_WhenButtonActivityDetected) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);

	prepare();
	step(1);
	LONGS_EQUAL(true, button_busy(button));
	finish();
}

TEST(ButtonElapsed, busy_ShouldReturnFalse_WhenNoActivityOnButtonDetected) {
	mock().expectNCalls(5, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);

	prepare();
	step(5+6+1);
	LONGS_EQUAL(false, button_busy(button));
	finish();
}

TEST(ButtonElapsed, busy_ShouldReturnFalse_WhenButtonStateIsLow) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);

	prepare();
	step(1);
	LONGS_EQUAL(false, button_busy(button));
	finish();
}

TEST(ButtonElapsed, busy_ShouldReturnTrue_WhenButtonStateIsHigh) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);

	prepare();
	step(1);
	LONGS_EQUAL(true, button_busy(button));
	finish();
}

TEST(ButtonElapsed, busy_ShouldReturnFalse_WhenDebouncingFiltered) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);

	prepare();
	step(1+6+1);
	LONGS_EQUAL(false, button_busy(button));
	finish();
}

TEST(ButtonElapsed, busy_ShouldReturnFalse_WhenClickWindowExpired) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(56, "get_button_state").andReturnValue(0);

	mock().expectNCalls(1, "on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);
	mock().expectNCalls(1, "on_button_event")
		.withParameter("event", BUTTON_EVT_RELEASED)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_CLICK)
		.withParameter("clicks", 1);

	prepare();
	step(6+56);
	LONGS_EQUAL(false, button_busy(button));
	finish();
}

TEST(ButtonElapsed, enable_ShouldReturnError_WhenNullGiven) {
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_enable(0));
}

TEST(ButtonElapsed, disable_ShouldReturnError_WhenNullGiven) {
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_disable(0));
}

TEST(ButtonElapsed, set_param_ShouldReturnError_WhenNullGiven) {
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_set_param(0, 0));
}

TEST(ButtonElapsed, get_param_ShouldReturnError_WhenNullGiven) {
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_get_param(0, 0));
}

TEST(ButtonElapsed, get_param_ShouldReturnParam_WhenValidButtonGiven) {
	struct button_param param = {0};
	prepare();
	LONGS_EQUAL(BUTTON_ERROR_NONE, button_get_param(button, &param));
	finish();
}

TEST(ButtonElapsed, set_param_ShouldReturnError_WhenInCorrectParamGiven) {
	struct button_param param = {0};
	prepare();
	LONGS_EQUAL(BUTTON_ERROR_INCORRECT_PARAM, button_set_param(button, &param));
	finish();
}

TEST(ButtonElapsed, step_ShouldReturnError_WhenNullGiven) {
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_step_elapsed(0, 0));
}

TEST(ButtonElapsed, step_ShouldReturnError_WhenNotEnabled) {
	button = button_new(get_button_state, 0, on_button_event, 0);
	LONGS_EQUAL(BUTTON_ERROR_DISABLED, button_step_elapsed(button, 0));
	button_delete(button);
}

TEST(ButtonElapsed, step_ShouldDoNothing_WhenCalledDelayedAfterNoiseGiven) {
	mock().expectNCalls(3, "get_button_state").andReturnValue(1);
	mock().expectNCalls(2, "get_button_state").andReturnValue(0);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);

	mock().expectNoCall("on_button_event");

	prepare();
	step(3+2);
	button_step_elapsed(button, BUTTON_MIN_PRESS_TIME_MS);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandlePressed_WhenCalledDelayedAfterHighGiven) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);

	prepare();
	step(1);
	button_step_elapsed(button, BUTTON_MIN_PRESS_TIME_MS);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleHolding_WhenCalledDelayed) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(30, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.withParameter("clicks", 0);

	prepare();
	step(6);
	button_step_elapsed(button, BUTTON_SAMPLING_INTERVAL_MS * 30);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleHoldingRepeat_WhenCalledDelayed) {
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);
	mock().expectNCalls(20, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.withParameter("clicks", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.withParameter("clicks", 0);

	prepare();
	step(36);
	button_step_elapsed(button, BUTTON_SAMPLING_INTERVAL_MS * 20);
	finish();
}
