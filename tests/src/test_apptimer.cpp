#include "CppUTest/TestHarness.h"

#include "libmcu/apptimer.h"
#include "logger.h"

static int nr_called;

static void callback(void *param)
{
	nr_called++;
}

TEST_GROUP(AppTimer) {
	void setup(void) {
		nr_called = 0;
		apptimer_init(NULL);
	}
	void teardown(void) {
		apptimer_deinit();
	}
};

TEST(AppTimer, create_ShouldReturnNull_WhenNullTimerGiven) {
	POINTERS_EQUAL(NULL, apptimer_create_static(NULL, false, NULL));
}

TEST(AppTimer, create_ShouldReturnNull_WhenNullCallbackGiven) {
	apptimer_t timer;
	POINTERS_EQUAL(NULL, apptimer_create_static(&timer, false, NULL));
}

TEST(AppTimer, create_ShouldReturnTheSamePointer_WhenTimerPointerGiven) {
	apptimer_t timer;
	POINTERS_EQUAL(&timer, apptimer_create_static(&timer, false, callback));
}

TEST(AppTimer, start_ShouldReturnInvalidParam_WhenNullTimerGiven) {
	LONGS_EQUAL(APPTIMER_INVALID_PARAM, apptimer_start(NULL, 10, NULL));
}

TEST(AppTimer, start_ShouldReturnAlreadyStarted_WhenStartAgain) {
	apptimer_t timer;
	apptimer_create_static(&timer, false, callback);
	LONGS_EQUAL(0, apptimer_count());
	LONGS_EQUAL(APPTIMER_SUCCESS, apptimer_start(&timer, 10, NULL));
	LONGS_EQUAL(APPTIMER_ALREADY_STARTED, apptimer_start(&timer, 10, NULL));
	LONGS_EQUAL(1, apptimer_count());
}

TEST(AppTimer, schedule_ShouldUpdateTimerWheels_WhenEverytimeCalled) {
	apptimer_t timer;
	apptimer_create_static(&timer, false, callback);
	apptimer_schedule(1);
	apptimer_start(&timer, 10, NULL);
	apptimer_schedule(1);
	apptimer_schedule(1);
	apptimer_schedule(1);
	apptimer_schedule(1);
	apptimer_schedule(1);
	apptimer_schedule(1);
	apptimer_schedule(1);
	apptimer_schedule(1);
	apptimer_schedule(1);
	LONGS_EQUAL(0, nr_called);
	apptimer_schedule(1);
	LONGS_EQUAL(1, nr_called);
}

TEST(AppTimer, start_ShouldAddTimerInTheRightWheel_WhenTimeCounterUpdated) {
	apptimer_t timer;
	apptimer_create_static(&timer, false, callback);
	apptimer_schedule(1);
	apptimer_start(&timer, 10, NULL);
	apptimer_schedule(9);
	LONGS_EQUAL(0, nr_called);
	apptimer_schedule(1);
	LONGS_EQUAL(1, nr_called);
}

TEST(AppTimer, Timer_ShouldExpire_WhenTimePassedMoreThanGivenTimeout) {
	apptimer_t timer;
	apptimer_create_static(&timer, false, callback);
	apptimer_schedule(17);
	apptimer_start(&timer, 10, NULL);
	apptimer_schedule(5);
	LONGS_EQUAL(0, nr_called);
	apptimer_schedule(10);
	LONGS_EQUAL(1, nr_called);
}

TEST(AppTimer, Timers_ShouldExpire_WhenTimePassedMoreThanGivenTimeout) {
	apptimer_t timer[16];
	apptimer_timeout_t tout = 2;
	int n = sizeof(timer) / sizeof(timer[0]);

	for (int i = 0; i < n; i++) {
		apptimer_create_static(&timer[i], false, callback);
		apptimer_start(&timer[i], tout, NULL);
		tout *= 2;
	}

	LONGS_EQUAL(n, apptimer_count());
	apptimer_schedule(tout);
	LONGS_EQUAL(n, nr_called);
	LONGS_EQUAL(0, apptimer_count());
}

TEST(AppTimer, ShouldCallCallback_WhenTimedOut) {
	apptimer_t timer[16];
	apptimer_timeout_t tout = 2;
	apptimer_timeout_t elapsed = 0;
	int n = sizeof(timer) / sizeof(timer[0]);

	for (int i = 0; i < n; i++) {
		apptimer_create_static(&timer[i], false, callback);
		apptimer_start(&timer[i], tout, NULL);
		tout *= 2;
	}

	LONGS_EQUAL(n, apptimer_count());
	for (int i = 0, tout = 2; i < n; i++) {
		debug("=> %lu:%lu #%lu", tout, elapsed, tout-elapsed);
		apptimer_schedule(tout - elapsed);
		LONGS_EQUAL(i+1, nr_called);
		LONGS_EQUAL(n-i-1, apptimer_count());
		elapsed += tout - elapsed;
		tout *= 2;
	}
}

static void hardware_timer_callback(apptimer_timeout_t t)
{
	apptimer_schedule(t);
}

TEST_GROUP(AppTimer_WithHardwareTimer) {
	void setup(void) {
		nr_called = 0;
		apptimer_init(hardware_timer_callback);
	}
	void teardown(void) {
		apptimer_deinit();
	}
};

TEST(AppTimer_WithHardwareTimer, tmp) {
}
