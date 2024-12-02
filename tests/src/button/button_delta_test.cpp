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

static void on_button_event(struct button *btn, const button_state_t event,
		const uint16_t clicks, const uint16_t repeats, void *ctx)
{
	mock().actualCall(__func__)
		.withParameter("event", event)
		.withParameter("clicks", clicks)
		.withParameter("repeats", repeats);
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
			LONGS_EQUAL(BUTTON_ERROR_NONE, button_step_delta(button, BUTTON_SAMPLING_PERIOD_MS));
		}
	}
};

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
	mock().expectNCalls(7, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	step(2+1+1+1+7);
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
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	step(6+4+2+1+1+1+13);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleReleased_WhenButtonReleased) {
	mock().expectNCalls(7, "get_button_state").andReturnValue(1);
	mock().expectNCalls(7, "get_button_state").andReturnValue(0);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_RELEASED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	step(7+7);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleHolding_WhenButtonKeepPressed) {
	mock().expectNCalls(2, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(37, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_HOLDING)
		.withParameter("clicks", 1)
		.withParameter("repeats", 1);

	prepare();
	step(2+1+1+1+37);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleHoldingRepeat_WhenButtonKeepPressedLonger) {
	mock().expectNCalls(37, "get_button_state").andReturnValue(1);
	mock().expectNCalls(20, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_HOLDING)
		.withParameter("clicks", 1)
		.withParameter("repeats", 1);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_HOLDING)
		.withParameter("clicks", 1)
		.withParameter("repeats", 2);

	prepare();
	step(37+20);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleHoldingRepeat_WhenButtonKeepPressedMuchLonger) {
	mock().expectNCalls(37, "get_button_state").andReturnValue(1);
	mock().expectNCalls(200, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_HOLDING)
		.withParameter("clicks", 1)
		.withParameter("repeats", 1);
	for (int i = 0; i < 10; i++) {
		mock().expectOneCall("on_button_event")
			.withParameter("event", BUTTON_STATE_HOLDING)
			.withParameter("clicks", 1)
			.withParameter("repeats", i+2);
	}

	prepare();
	step(37+200);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleClick_WhenTwoClickGiven) {
	mock().expectNCalls(7, "get_button_state").andReturnValue(1);
	mock().expectNCalls(7, "get_button_state").andReturnValue(0);
	mock().expectNCalls(7, "get_button_state").andReturnValue(1);
	mock().expectNCalls(7, "get_button_state").andReturnValue(0);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_RELEASED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 2)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_RELEASED)
		.withParameter("clicks", 2)
		.withParameter("repeats", 0);

	prepare();
	step(7+7+7+7);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleClick_WhenOneClickGivenTwice) {
	mock().expectNCalls(7, "get_button_state").andReturnValue(1);
	mock().expectNCalls(57, "get_button_state").andReturnValue(0);

	mock().expectNCalls(7, "get_button_state").andReturnValue(1);
	mock().expectNCalls(57, "get_button_state").andReturnValue(0);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_RELEASED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_RELEASED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	step(7+57+7+57);
	finish();
}

TEST(ButtonElapsed, step_ShouldReturnError_WhenNullGiven) {
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_step_delta(0, 0));
}

TEST(ButtonElapsed, step_ShouldReturnError_WhenNotEnabled) {
	button = button_new(get_button_state, 0, on_button_event, 0);
	LONGS_EQUAL(BUTTON_ERROR_DISABLED, button_step_delta(button, 0));
	button_delete(button);
}

TEST(ButtonElapsed, step_ShouldDoNothing_WhenCalledDelayedAfterNoiseGiven) {
	mock().expectNCalls(3, "get_button_state").andReturnValue(1);
	mock().expectNCalls(2, "get_button_state").andReturnValue(0);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);

	mock().expectNoCall("on_button_event");

	prepare();
	step(3+2);
	button_step_delta(button, BUTTON_DEBOUNCE_DURATION_MS);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandlePressed_WhenCalledDelayedAfterHighGiven) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	step(1);
	button_step_delta(button, BUTTON_DEBOUNCE_DURATION_MS);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleHolding_WhenCalledDelayed) {
	mock().expectNCalls(7, "get_button_state").andReturnValue(1);
	mock().expectNCalls(30, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_HOLDING)
		.withParameter("clicks", 1)
		.withParameter("repeats", 1);

	prepare();
	step(7);
	button_step_delta(button, BUTTON_SAMPLING_PERIOD_MS * 30);
	finish();
}

TEST(ButtonElapsed, step_ShouldHandleHoldingRepeat_WhenCalledDelayed) {
	mock().expectNCalls(37, "get_button_state").andReturnValue(1);
	mock().expectNCalls(20, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_HOLDING)
		.withParameter("clicks", 1)
		.withParameter("repeats", 1);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_HOLDING)
		.withParameter("clicks", 1)
		.withParameter("repeats", 2);

	prepare();
	step(37);
	button_step_delta(button, BUTTON_SAMPLING_PERIOD_MS * 20);
	finish();
}

TEST(ButtonElapsed, step_ShouldSkipHoldingEvent_WhenCalledDelayed) {
	mock().expectNCalls(37, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	button_step_delta(button, BUTTON_SAMPLING_PERIOD_MS * 37);
	finish();
}

TEST(ButtonElapsed, state_ShouldReturnPressed_WhenButtonPressed) {
	mock().expectNCalls(7, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	button_step_delta(button, BUTTON_SAMPLING_PERIOD_MS * 7);
	LONGS_EQUAL(BUTTON_STATE_PRESSED, button_state(button));
	finish();
}

TEST(ButtonElapsed, state_ShouldReturnHolding_WhenButtonHolding) {
	mock().expectNCalls(7, "get_button_state").andReturnValue(1);
	mock().expectNCalls(30, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_HOLDING)
		.withParameter("clicks", 1)
		.withParameter("repeats", 1);

	prepare();
	button_step_delta(button, BUTTON_SAMPLING_PERIOD_MS * 7);
	button_step_delta(button, BUTTON_SAMPLING_PERIOD_MS * 30);
	LONGS_EQUAL(BUTTON_STATE_HOLDING, button_state(button));
	finish();
}

TEST(ButtonElapsed, state_ShouldReturnReleased_WhenButtonReleased) {
	mock().expectNCalls(7, "get_button_state").andReturnValue(1);
	mock().expectNCalls(7, "get_button_state").andReturnValue(0);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_RELEASED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	button_step_delta(button, BUTTON_SAMPLING_PERIOD_MS * 7);
	button_step_delta(button, BUTTON_SAMPLING_PERIOD_MS * 7);
	LONGS_EQUAL(BUTTON_STATE_RELEASED, button_state(button));
	finish();
}
