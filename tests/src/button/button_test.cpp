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

static int get_button_state(void)
{
	return mock().actualCall(__func__).returnIntValueOrDefault(0);
}

static void on_button_event(enum button_event event,
		const struct button_data *info, void *ctx)
{
	mock().actualCall(__func__)
		.withParameter("event", event)
		.withParameterOfType("clickType", "info", info);
}

static unsigned long fake_get_time_ms(void)
{
	return mock().actualCall(__func__).returnUnsignedLongIntValueOrDefault(0);
}

class ClickComparator : public MockNamedValueComparator
{
public:
	virtual bool isEqual(const void* object1, const void* object2) {
		const struct button_data *p1 = (const struct button_data *)object1;
		const struct button_data *p2 = (const struct button_data *)object2;
		return p1->click == p2->click;
	}
	virtual SimpleString valueToString(const void* object) {
		return StringFrom(object);
	}
};

TEST_GROUP(button) {
	ClickComparator clickComp;

	void setup(void) {
		mock().installComparator("clickType", clickComp);

		button_init(fake_get_time_ms);
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().removeAllComparatorsAndCopiers();
		mock().clear();
	}

	button_rc_t step(int n) {
		unsigned long t = 0;
		button_rc_t rc;

		for (int i = 0; i < n; i++) {
			do {
				mock().expectOneCall("fake_get_time_ms")
					.andReturnValue(t++);
			} while ((rc = button_step()) == BUTTON_BUSY) ;
		}

		return rc;
	}
};

TEST(button, register_ShouldReturnFalse_WhenInvalidParamsGiven) {
	int tmp;
	LONGS_EQUAL(0, button_register(NULL, NULL, NULL));
	LONGS_EQUAL(0, button_register(NULL, on_button_event, NULL));
	LONGS_EQUAL(0, button_register(NULL, on_button_event, &tmp));
}

TEST(button, register_ShouldReturnNull_WhenNoSlotsLeft) {
	CHECK(button_register(get_button_state, on_button_event, 0) != NULL);
	CHECK(button_register(get_button_state, on_button_event, 0) != NULL);
	CHECK(button_register(get_button_state, on_button_event, 0) != NULL);
	CHECK(button_register(get_button_state, on_button_event, 0) == NULL);
}

TEST(button, register_ShouldReturnHandle) {
	CHECK(button_register(get_button_state, on_button_event, 0) != NULL);
}

TEST(button, step_ShouldDoNothing_WhenNoiseGiven) {
	//0x1cf1f: 0b1_1100_1111_0001_1111
	mock().expectNCalls(3, "get_button_state").andReturnValue(1);
	mock().expectNCalls(2, "get_button_state").andReturnValue(0);
	mock().expectNCalls(4, "get_button_state").andReturnValue(1);
	mock().expectNCalls(3, "get_button_state").andReturnValue(0);
	mock().expectNCalls(5, "get_button_state").andReturnValue(1);
	mock().expectNCalls(7, "get_button_state").andReturnValue(0);

	mock().expectNoCall("on_button_event");

	button_register(get_button_state, on_button_event, 0);

	step(3+2+4+3+5+7);
}

TEST(button, step_ShouldHandlePressed_WhenValidPatternGiven) {
	//0x35fff: 0b11_0101_1111_1111_1111
	mock().expectNCalls(2, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.ignoreOtherParameters();

	button_register(get_button_state, on_button_event, 0);

	step(2+1+1+1+6);
}

TEST(button, step_ShouldIgnoreNoise_WhenNoiseGivenInMiddleOfPressed) {
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
		.ignoreOtherParameters();

	button_register(get_button_state, on_button_event, 0);

	step(6+4+2+1+1+1+13);
}

TEST(button, step_ShouldHandleReleased_WhenButtonReleased) {
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.ignoreOtherParameters();
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_RELEASED)
		.ignoreOtherParameters();
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_CLICK)
		.ignoreOtherParameters();

	button_register(get_button_state, on_button_event, 0);

	step(6+6);
}

TEST(button, step_ShouldCallHolding_WhenHoldingButtonPressed) {
	mock().expectNCalls(2, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.ignoreOtherParameters();
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.ignoreOtherParameters();

	button_register(get_button_state, on_button_event, 0);

	step(2+1+1+1+36);
}

TEST(button, poll_ShouldCallHoldingRepeat_WhenButtonPressedKept) {
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);
	mock().expectNCalls(10, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.ignoreOtherParameters();
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.ignoreOtherParameters();
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.ignoreOtherParameters();

	button_register(get_button_state, on_button_event, 0);

	step(36+10);
}

TEST(button, poll_ShouldCallHoldingRepeat_WhenButtonPressedKeptLonger) {
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);
	mock().expectNCalls(100, "get_button_state").andReturnValue(1);

	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.ignoreOtherParameters();
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.ignoreOtherParameters();
	mock().expectNCalls(10, "on_button_event")
		.withParameter("event", BUTTON_EVT_HOLDING)
		.ignoreOtherParameters();

	button_register(get_button_state, on_button_event, 0);

	step(36+100);
}

TEST(button, step_ShouldHandleClick_WhenTwoClickGiven) {
	struct button_data oneClick = { .click = 1, };
	struct button_data twoClicks = { .click = 2, };

	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);
	mock().expectNCalls(6, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);

	mock().expectNCalls(2, "on_button_event")
		.withParameter("event", BUTTON_EVT_PRESSED)
		.ignoreOtherParameters();
	mock().expectNCalls(2, "on_button_event")
		.withParameter("event", BUTTON_EVT_RELEASED)
		.ignoreOtherParameters();
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_CLICK)
		.withParameterOfType("clickType", "info", &oneClick);
	mock().expectOneCall("on_button_event")
		.withParameter("event", BUTTON_EVT_CLICK)
		.withParameterOfType("clickType", "info", &twoClicks);

	button_register(get_button_state, on_button_event, 0);

	step(6+6+6+6);
}

TEST(button, step_ShouldReturnScanning_WhenButtonActivityDetected) {
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectOneCall("fake_get_time_ms").andReturnValue(10);

	button_register(get_button_state, on_button_event, 0);

	CHECK(button_step() == BUTTON_SCANNING);
}

TEST(button, step_ShouldReturnNoActivity_WhenNoActivityOnButtonDetected) {
	mock().expectNCalls(5, "get_button_state").andReturnValue(1);
	mock().expectNCalls(6, "get_button_state").andReturnValue(0);

	button_register(get_button_state, on_button_event, 0);

	step(5);

	for (unsigned int i = 0; i < 5; i++) {
		mock().expectOneCall("fake_get_time_ms").andReturnValue(100 + i * 10);
		CHECK(button_step() == BUTTON_SCANNING);
	}

	mock().expectOneCall("fake_get_time_ms").andReturnValue(100 + 5 * 10);
	CHECK(button_step() == BUTTON_NO_ACTIVITY);
}

TEST(button, step_ShouldReturnBusy_WhenCallingManyInOnePeriod) {
	mock().expectOneCall("fake_get_time_ms").andReturnValue(9);

	button_register(get_button_state, on_button_event, 0);

	CHECK(button_step() == BUTTON_BUSY);
}
