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

static void pressed(const struct button_data *btn, void *context)
{
	mock().actualCall(__func__);
}

static void released(const struct button_data *btn, void *context)
{
	mock().actualCall(__func__);
}

static void holding(const struct button_data *btn, void *context)
{
	mock().actualCall(__func__);
}

static unsigned long fake_time;

static unsigned long fake_get_time_ms(void)
{
	return fake_time++;
}

TEST_GROUP(button) {
	struct button_handlers handlers;

	void setup(void) {
		mock().ignoreOtherCalls();

		button_init(fake_get_time_ms);
		handlers.pressed = pressed;
		handlers.released = released;
		handlers.holding = holding;
		fake_time = (unsigned int)-33;
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(button, register_ShouldReturnFalse_WhenInvalidParamsGiven) {
	LONGS_EQUAL(0, button_register(NULL, NULL));
	LONGS_EQUAL(0, button_register(&handlers, NULL));
	LONGS_EQUAL(0, button_register(NULL, get_button_state));
}

TEST(button, register_ShouldReturnNull_WhenNoSlotsLeft) {
	CHECK(button_register(&handlers, get_button_state) != NULL);
	CHECK(button_register(&handlers, get_button_state) != NULL);
	CHECK(button_register(&handlers, get_button_state) != NULL);
	CHECK(button_register(&handlers, get_button_state) == NULL);
}

TEST(button, register_ShouldReturnHandle) {
	CHECK(button_register(&handlers, get_button_state) != NULL);
}

TEST(button, poll_ShouldDoNothing_WhenNoiseGiven) {
	//0x1cf1f: 0b1_1100_1111_0001_1111
	mock().expectNCalls(3, "get_button_state").andReturnValue(1);
	mock().expectNCalls(2, "get_button_state").andReturnValue(0);
	mock().expectNCalls(4, "get_button_state").andReturnValue(1);
	mock().expectNCalls(3, "get_button_state").andReturnValue(0);
	mock().expectNCalls(5, "get_button_state").andReturnValue(1);
	mock().expectNCalls(7, "get_button_state").andReturnValue(0);

	mock().expectNoCall("pressed");
	mock().expectNoCall("holding");
	mock().expectNoCall("released");

	button_register(&handlers, get_button_state);

	int n = 3+2+4+3+5+7;
	for (int i = 0; i < n; i++) {
		while (!button_poll(NULL)) ;
	}
}

TEST(button, poll_ShouldCallPressed_WhenValidPatternGiven) {
	//0x35fff: 0b11_0101_1111_1111_1111
	mock().expectNCalls(2, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(13, "get_button_state").andReturnValue(1);
	mock().expectNCalls(7, "get_button_state").andReturnValue(0);

	mock().expectOneCall("pressed");
	mock().expectNoCall("holding");
	mock().expectOneCall("released");

	button_register(&handlers, get_button_state);

	int n = 2+1+1+1+13+7;
	for (int i = 0; i < n; i++) {
		while (!button_poll(NULL)) ;
	}
}

TEST(button, poll_ShouldIgnoreNoise_WhenNoiseGivenInMiddleOfPressed) {
	//0xffc35fff: 0b1111_1111_1100_0011_0101_1111_1111_1111
	mock().expectNCalls(10, "get_button_state").andReturnValue(1);
	mock().expectNCalls(4, "get_button_state").andReturnValue(0);
	mock().expectNCalls(2, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(13, "get_button_state").andReturnValue(1);
	mock().expectNCalls(7, "get_button_state").andReturnValue(0);

	mock().expectOneCall("pressed");
	mock().expectNoCall("holding");
	mock().expectOneCall("released");

	button_register(&handlers, get_button_state);

	int n = 10+4+2+1+1+1+13+7;
	for (int i = 0; i < n; i++) {
		while (!button_poll(NULL)) ;
	}
}

TEST(button, poll_ShouldCallHolding_WhenHoldingButtonPressed) {
	mock().expectNCalls(2, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(1, "get_button_state").andReturnValue(1);
	mock().expectNCalls(1, "get_button_state").andReturnValue(0);
	mock().expectNCalls(36, "get_button_state").andReturnValue(1);
	mock().expectNCalls(7, "get_button_state").andReturnValue(0);

	mock().expectOneCall("pressed");
	mock().expectOneCall("holding");
	mock().expectOneCall("released");

	button_register(&handlers, get_button_state);

	int n = 2+1+1+1+36+7;
	for (int i = 0; i < n; i++) {
		while (!button_poll(NULL)) ;
	}
}

TEST(button, is_pressed_ShouldReturnTrue_WhenStateIsFluctuate) {
	mock().expectOneCall("get_button_state").andReturnValue(1);
	const void *btn_handle = button_register(&handlers, get_button_state);
	while (!button_poll(NULL)) ;
	LONGS_EQUAL(1, button_is_pressed(btn_handle));
}

TEST(button, is_pressed_ShouldReturnFalse_WhenButtonIsUp) {
	mock().expectOneCall("get_button_state").andReturnValue(0);
	const void *btn_handle = button_register(&handlers, get_button_state);
	while (!button_poll(NULL)) ;
	LONGS_EQUAL(0, button_is_pressed(btn_handle));
}
