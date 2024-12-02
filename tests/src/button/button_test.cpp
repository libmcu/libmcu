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

TEST_GROUP(Button) {
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
		const uint32_t milliseconds = count * BUTTON_SAMPLING_PERIOD_MS;
		for (uint32_t i = 0; i <= milliseconds; i++) {
			LONGS_EQUAL(BUTTON_ERROR_NONE, button_step(button, i));
		}
	}
};

TEST(Button, new_ShouldReturnNULL_WhenNoGetStateGiven) {
	LONGS_EQUAL(0, button_new(NULL, NULL, NULL, NULL));
}

TEST(Button, new_ShouldReturnNull_WhenNoSlotsLeft) {
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

TEST(Button, new_ShouldReturnInstance_WhenFreeSlotAvailableAfterFull) {
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

TEST(Button, new_ShouldReturnInstance_WhenValidArgumentsGiven) {
	struct button *p = button_new(get_button_state, 0, on_button_event, 0);
	CHECK(p != NULL);
	button_delete(p);
}

TEST(Button, step_ShouldDoNothing_WhenNoiseGiven) {
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

TEST(Button, step_ShouldHandlePressed_WhenValidPatternGiven) {
	//0x35fff: 0b11_0101_1111_1111_1111
	mock().expectNCalls(2, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	step(2+1+1+1+6);
	finish();
}

TEST(Button, step_ShouldIgnoreNoise_WhenNoiseGivenInMiddleOfPressed) {
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

TEST(Button, step_ShouldHandleReleased_WhenButtonReleased) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_RELEASED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	step(6+6);
	finish();
}

TEST(Button, step_ShouldHandleHolding_WhenButtonKeepPressed) {
	mock().expectNCalls(2, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_HOLDING)
		.withParameter("clicks", 1)
		.withParameter("repeats", 1);

	prepare();
	step(2+1+1+1+36);
	finish();
}

TEST(Button, step_ShouldHandleHoldingRepeat_WhenButtonKeepPressedLonger) {
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);
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
	step(36+20);
	finish();
}

TEST(Button, step_ShouldHandleHoldingRepeat_WhenButtonKeepPressedMuchLonger) {
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);
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
	step(36+200);
	finish();
}

TEST(Button, step_ShouldHandleClick_WhenTwoClickGiven) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);

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
	step(6+6+6+6);
	finish();
}

TEST(Button, step_ShouldHandleClick_WhenOneClickGivenTwice) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(56, "get_button_state").andReturnValue(0);

	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(56, "get_button_state").andReturnValue(0);

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
	step(6+56+6+56);
	finish();
}

TEST(Button, busy_ShouldReturnFalse_WhenInitialized) {
	prepare();
	LONGS_EQUAL(false, button_busy(button));
	finish();
}

TEST(Button, busy_ShouldReturnTrue_WhenButtonActivityDetected) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);

	prepare();
	step(1);
	LONGS_EQUAL(true, button_busy(button));
	finish();
}

TEST(Button, busy_ShouldReturnFalse_WhenNoActivityOnButtonDetected) {
	mock().expectNCalls(5, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);

	prepare();
	step(5+6+1);
	LONGS_EQUAL(false, button_busy(button));
	finish();
}

TEST(Button, busy_ShouldReturnFalse_WhenButtonStateIsLow) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);

	prepare();
	step(1);
	LONGS_EQUAL(false, button_busy(button));
	finish();
}

TEST(Button, busy_ShouldReturnTrue_WhenButtonStateIsHigh) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);

	prepare();
	step(1);
	LONGS_EQUAL(true, button_busy(button));
	finish();
}

TEST(Button, busy_ShouldReturnFalse_WhenDebouncingFiltered) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);

	prepare();
	step(1+6+1);
	LONGS_EQUAL(false, button_busy(button));
	finish();
}

TEST(Button, busy_ShouldReturnFalse_WhenClickWindowExpired) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(56, "get_button_state").andReturnValue(0);

	mock().expectNCalls(1, "on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);
	mock().expectNCalls(1, "on_button_event")
		.withParameter("event", BUTTON_STATE_RELEASED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	step(6+56);
	LONGS_EQUAL(false, button_busy(button));
	finish();
}

TEST(Button, enable_ShouldReturnError_WhenNullGiven) {
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_enable(0));
}

TEST(Button, disable_ShouldReturnError_WhenNullGiven) {
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_disable(0));
}

TEST(Button, set_param_ShouldReturnError_WhenNullGiven) {
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_set_param(0, 0));
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_set_param(button, 0));
}

TEST(Button, get_param_ShouldReturnError_WhenNullGiven) {
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_get_param(0, 0));
}

TEST(Button, get_param_ShouldReturnParam_WhenValidButtonGiven) {
	struct button_param param = {0};
	prepare();
	LONGS_EQUAL(BUTTON_ERROR_NONE, button_get_param(button, &param));
	finish();
}

TEST(Button, set_param_ShouldReturnError_WhenInCorrectParamGiven) {
	struct button_param param = {0};
	prepare();
	LONGS_EQUAL(BUTTON_ERROR_INCORRECT_PARAM, button_set_param(button, &param));
	finish();
}

TEST(Button, set_param_ShouldSetMaxSamplingInterval_WhenZeroValueOfItGiven) {
	struct button_param param;
	prepare();
	button_get_param(button, &param);
	param.max_sampling_interval_ms = 0;
	LONGS_EQUAL(BUTTON_ERROR_NONE, button_set_param(button, &param));
	finish();
}

TEST(Button, set_param_ShouldReturnError_WhenSamplingIntervalIsZero) {
	struct button_param param;
	prepare();
	button_get_param(button, &param);
	param.sampling_interval_ms = 0;
	LONGS_EQUAL(BUTTON_ERROR_INCORRECT_PARAM, button_set_param(button, &param));
	finish();
}

TEST(Button, set_param_ShouldReturnError_WhenMinPressTimeIsLessThanSamplingInterval) {
	struct button_param param;
	prepare();
	button_get_param(button, &param);
	param.min_press_time_ms = param.sampling_interval_ms - 1;
	LONGS_EQUAL(BUTTON_ERROR_INCORRECT_PARAM, button_set_param(button, &param));
	finish();
}

TEST(Button, set_param_ShouldReturnError_WhenWaveformIsTooLong) {
	struct button_param param;
	prepare();
	button_get_param(button, &param);
	param.min_press_time_ms = 30;
	param.sampling_interval_ms = 1;
	LONGS_EQUAL(BUTTON_ERROR_INCORRECT_PARAM, button_set_param(button, &param));
	finish();
}

TEST(Button, step_ShouldReturnError_WhenNullGiven) {
	LONGS_EQUAL(BUTTON_ERROR_INVALID_PARAM, button_step(0, 0));
}

TEST(Button, step_ShouldReturnError_WhenNotEnabled) {
	button = button_new(get_button_state, 0, on_button_event, 0);
	LONGS_EQUAL(BUTTON_ERROR_DISABLED, button_step(button, 0));
	button_delete(button);
}

TEST(Button, step_ShouldDoNothing_WhenCalledDelayedAfterNoiseGiven) {
	mock().expectNCalls(3, "get_button_state").andReturnValue(1);
	mock().expectNCalls(2, "get_button_state").andReturnValue(0);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);

	mock().expectNoCall("on_button_event");

	prepare();
	step(3+2);
	const uint32_t elapsed = (3+2)*BUTTON_SAMPLING_PERIOD_MS;
	button_step(button, elapsed + BUTTON_DEBOUNCE_DURATION_MS);
	finish();
}

TEST(Button, step_ShouldHandlePressed_WhenCalledDelayedAfterHighGiven) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	prepare();
	step(1);
	const uint32_t elapsed = (1)*BUTTON_SAMPLING_PERIOD_MS;
	button_step(button, elapsed + BUTTON_DEBOUNCE_DURATION_MS);
	finish();
}

TEST(Button, step_ShouldHandleHolding_WhenCalledDelayed) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
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
	step(6);
	const uint32_t elapsed = (6)*BUTTON_SAMPLING_PERIOD_MS;
	button_step(button, elapsed + BUTTON_SAMPLING_PERIOD_MS * 30);
	finish();
}

TEST(Button, step_ShouldHandleHoldingRepeat_WhenCalledDelayed) {
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);
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
	step(36);
	const uint32_t elapsed = (36)*BUTTON_SAMPLING_PERIOD_MS;
	button_step(button, elapsed + BUTTON_SAMPLING_PERIOD_MS * 20);
	finish();
}

TEST(Button, step_ShouldSyncItsTime_WhenCalledForTheFirstTime) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	prepare();
	button_step(button, 1001);
	finish();
}

TEST(Button, step_ShouldKeepThePreviousState_WhenCalledAfterWhild) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_PRESSED)
		.withParameter("clicks", 1)
		.withParameter("repeats", 0);

	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_STATE_HOLDING)
		.withParameter("clicks", 1)
		.withParameter("repeats", 1);

	prepare();
	step(6);
	button_step(button, 10000);
	finish();
}

TEST(Button, clicks_ShouldReturnNumberOfClicks_WhenCalled) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
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

	prepare();
	LONGS_EQUAL(0, button_clicks(button));
	step(6);
	LONGS_EQUAL(1, button_clicks(button));
	const uint32_t elapsed = (6)*BUTTON_SAMPLING_PERIOD_MS;
	for (uint32_t i = 1; i <= 12; i++) {
		button_step(button, elapsed + BUTTON_SAMPLING_PERIOD_MS * i);
	}
	LONGS_EQUAL(2, button_clicks(button));
	finish();
}

TEST(Button, repeats_ShouldReturnNumberOfRepeats_WhenCalled) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(30, "get_button_state").andReturnValue(1);
	mock().expectNCalls(30, "get_button_state").andReturnValue(1);
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
	step(6);
	LONGS_EQUAL(0, button_repeats(button));
	uint32_t elapsed = (6)*BUTTON_SAMPLING_PERIOD_MS;
	for (uint32_t i = 1; i <= 30; i++) {
		button_step(button, elapsed + BUTTON_SAMPLING_PERIOD_MS * i);
	}
	LONGS_EQUAL(1, button_repeats(button));
	elapsed += (30)*BUTTON_SAMPLING_PERIOD_MS;
	for (uint32_t i = 1; i <= 30; i++) {
		button_step(button, elapsed + BUTTON_SAMPLING_PERIOD_MS * i);
	}
	LONGS_EQUAL(2, button_repeats(button));
	finish();
}
