#include "CppUTest/TestHarness.h"
#include "libmcu/button.h"
#include "libmcu/compiler.h"

static unsigned int fake_time;
static unsigned int fake_state;
static int pressed_count;
static int released_count;

static int fake_get_state(void)
{
	int ret = (int)(fake_state & 1);
	fake_state >>= 1;
	return ret;
}

static void fake_delayf(unsigned int ms)
{
	fake_time += ms;
}

static unsigned int fake_get_time_ms(void)
{
	return fake_time++;
}

static void pressed(const struct button_data *btn, void *context)
{
	pressed_count++;
	unused(btn);
	unused(context);
}
static void released(const struct button_data *btn, void *context)
{
	released_count++;
	unused(btn);
	unused(context);
}

TEST_GROUP(button) {
	button_handlers_t handlers;

	void setup(void) {
		button_init(fake_get_time_ms, fake_delayf);
		handlers.pressed = pressed;
		handlers.released = released;
		fake_time = 0;
		fake_state = 0;
		pressed_count = 0;
		released_count = 0;
	}
	void teardown(void) {
	}
};

TEST(button, register_ShouldReturnFalse_WhenInvalidParamsGiven) {
	LONGS_EQUAL(0, button_register(NULL, NULL));
	LONGS_EQUAL(0, button_register(&handlers, NULL));
	LONGS_EQUAL(0, button_register(NULL, fake_get_state));
}

TEST(button, register_ShouldReturnFalse_WhenNoSlotsLeft) {
	LONGS_EQUAL(1, button_register(&handlers, fake_get_state));
	LONGS_EQUAL(1, button_register(&handlers, fake_get_state));
	LONGS_EQUAL(1, button_register(&handlers, fake_get_state));
	LONGS_EQUAL(0, button_register(&handlers, fake_get_state));
}

TEST(button, register_ShouldReturnTrue) {
	LONGS_EQUAL(1, button_register(&handlers, fake_get_state));
}

TEST(button, poll_ShouldDoNothing_WhenNoiseGiven) {
	fake_state = 0x1cf1f; // 0b1_1100_1111_0001_1111
	button_register(&handlers, fake_get_state);
	button_poll(NULL);
	LONGS_EQUAL(0, pressed_count);
	LONGS_EQUAL(0, released_count);
}

TEST(button, poll_ShouldCallPressed_WhenValidPatternGiven) {
	fake_state = 0x35fff; // 0b0011_0101_1111_1111_1111
	button_register(&handlers, fake_get_state);
	button_poll(NULL);
	LONGS_EQUAL(1, pressed_count);
	LONGS_EQUAL(1, released_count);
}

TEST(button, poll_ShouldIgnoreNoise_WhenNoiseGivenInMiddleOfPressed) {
	fake_state = 0xffc35fff; // 0b1111_1111_1100_0011_0101_1111_1111_1111
	button_register(&handlers, fake_get_state);
	button_poll(NULL);
	LONGS_EQUAL(1, pressed_count);
	LONGS_EQUAL(1, released_count);
}
