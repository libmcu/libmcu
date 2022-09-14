/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"

#include "libmcu/apptimer.h"

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
	apptimer_static_t timer;
	POINTERS_EQUAL(NULL, apptimer_create_static(&timer, false, NULL));
}

TEST(AppTimer, create_ShouldReturnTheSamePointer_WhenTimerPointerGiven) {
	apptimer_static_t timer;
	POINTERS_EQUAL(&timer, apptimer_create_static(&timer, false, callback));
}

TEST(AppTimer, create_dynamic_ShouldReturnNull) {
	POINTERS_EQUAL(NULL, apptimer_create(false, callback));
}

TEST(AppTimer, start_ShouldReturnInvalidParam_WhenNullTimerGiven) {
	LONGS_EQUAL(APPTIMER_INVALID_PARAM, apptimer_start(NULL, 10, NULL));
}

TEST(AppTimer, start_ShouldReturnAlreadyStarted_WhenStartAgain) {
	apptimer_static_t timer;
	apptimer_create_static(&timer, false, callback);
	LONGS_EQUAL(0, apptimer_count());
	LONGS_EQUAL(APPTIMER_SUCCESS, apptimer_start(&timer, 10, NULL));
	LONGS_EQUAL(APPTIMER_ALREADY_STARTED, apptimer_start(&timer, 10, NULL));
	LONGS_EQUAL(1, apptimer_count());
}

TEST(AppTimer, start_ShouldReturnTimeLimitExceeded_WhenMoreThanMaxTimeoutGiven) {
	apptimer_static_t timer;
	apptimer_create_static(&timer, false, callback);
	LONGS_EQUAL(APPTIMER_TIME_LIMIT_EXCEEDED,
			apptimer_start(&timer, APPTIMER_MAX_TIMEOUT+1, NULL));
}

TEST(AppTimer, start_ShouldRunTimerRecursively_WhenRepeatOptionGiven) {
	apptimer_static_t timer;
	apptimer_create_static(&timer, true, callback);
	LONGS_EQUAL(APPTIMER_SUCCESS, apptimer_start(&timer, 10, NULL));
	for (int i = 0; i < 10; i++) {
		LONGS_EQUAL(1, apptimer_count());
		apptimer_schedule(10);
		LONGS_EQUAL(i+1, nr_called);
	}
	apptimer_stop(&timer);
	LONGS_EQUAL(0, apptimer_count());
}

TEST(AppTimer, stop_ShouldStopTimerStarted) {
	apptimer_static_t timer;
	apptimer_create_static(&timer, false, callback);
	LONGS_EQUAL(APPTIMER_SUCCESS, apptimer_start(&timer, 10, NULL));
	LONGS_EQUAL(1, apptimer_count());
	LONGS_EQUAL(APPTIMER_SUCCESS, apptimer_stop(&timer));
	LONGS_EQUAL(0, apptimer_count());
}

IGNORE_TEST(AppTimer, stop_ShouldDoNothing_WhenAlreadStoppedTimerGiven) {
}

IGNORE_TEST(AppTimer, stop_ShouldDoNothing_WhenNotStartedTimerGiven) {
}

TEST(AppTimer, delete_ShouldDeleteTimerCreated) {
	apptimer_static_t timer;
	apptimer_create_static(&timer, false, callback);
	LONGS_EQUAL(APPTIMER_SUCCESS, apptimer_destroy(&timer));
}

IGNORE_TEST(AppTimer, delete_ShouldDoNothing_WhenAlreadyDeletedTimerGiven) {
}

TEST(AppTimer, schedule_ShouldUpdateTimerWheels_WhenEverytimeCalled) {
	apptimer_static_t timer;
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

TEST(AppTimer, schedule_ShouldNotCompensateTimerTimeout_WhenLatencyGiven) {
	apptimer_static_t timer;
	apptimer_create_static(&timer, true, callback);
	apptimer_start(&timer, 10, NULL);
	apptimer_schedule(15);
	LONGS_EQUAL(1, nr_called);
	//apptimer_schedule(5);
	apptimer_schedule(10);
	LONGS_EQUAL(2, nr_called);
}

TEST(AppTimer, schedule_ShouldRunTheSame_WhenCounterGoesWrapOver) {
	apptimer_static_t timer;
	apptimer_create_static(&timer, true, callback);
	apptimer_schedule(((apptimer_timeout_t)-1) - 5);
	apptimer_start(&timer, 10, NULL);
	apptimer_schedule(9);
	LONGS_EQUAL(0, nr_called);
	apptimer_schedule(1);
	LONGS_EQUAL(1, nr_called);
	apptimer_schedule(10);
	LONGS_EQUAL(2, nr_called);
	apptimer_schedule(13);
	LONGS_EQUAL(3, nr_called);
	apptimer_schedule(7);
	// NOTE: it does not compensate the latency
	// so even if timed out after 3ms delay, the next expiration will be
	// after `current time` + timeout + 3ms delay
	LONGS_EQUAL(3, nr_called);
	apptimer_schedule(3);
	LONGS_EQUAL(4, nr_called);
}

TEST(AppTimer, schedule_ShouldRunTheSame_WhenCounterGoesOverSignBit) {
	apptimer_static_t timer;
	apptimer_create_static(&timer, false, callback);
	// after APPTIMER_MAX_TIMEOUT+1, the sign bit of timer counter will be
	// set
	apptimer_schedule(APPTIMER_MAX_TIMEOUT - 5);
	apptimer_start(&timer, 10, NULL);
	apptimer_schedule(9);
	LONGS_EQUAL(0, nr_called);
	apptimer_schedule(1);
	LONGS_EQUAL(1, nr_called);
}

TEST(AppTimer, start_ShouldAddTimerInTheRightWheel_WhenTimeCounterUpdated) {
	apptimer_static_t timer;
	apptimer_create_static(&timer, false, callback);
	apptimer_schedule(1);
	apptimer_start(&timer, 10, NULL);
	apptimer_schedule(9);
	LONGS_EQUAL(0, nr_called);
	apptimer_schedule(1);
	LONGS_EQUAL(1, nr_called);
}

TEST(AppTimer, Timer_ShouldExpire_WhenTimePassedMoreThanGivenTimeout) {
	apptimer_static_t timer;
	apptimer_create_static(&timer, false, callback);
	apptimer_schedule(17);
	apptimer_start(&timer, 10, NULL);
	apptimer_schedule(5);
	LONGS_EQUAL(0, nr_called);
	apptimer_schedule(10);
	LONGS_EQUAL(1, nr_called);
}

TEST(AppTimer, Timers_ShouldExpire_WhenTimePassedMoreThanGivenTimeout) {
	apptimer_static_t timer[16];
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
	apptimer_static_t timer[16];
	apptimer_timeout_t tout = 2;
	apptimer_timeout_t elapsed = 0;
	int n = sizeof(timer) / sizeof(timer[0]);

	for (int i = 0; i < n; i++) {
		apptimer_create_static(&timer[i], false, callback);
		apptimer_start(&timer[i], tout, NULL);
		tout *= 2;
	}

	LONGS_EQUAL(n, apptimer_count());
	tout = 2;
	for (int i = 0; i < n; i++) {
		apptimer_schedule(tout - elapsed);
		LONGS_EQUAL(i+1, nr_called);
		LONGS_EQUAL(n-i-1, apptimer_count());
		elapsed += tout - elapsed;
		tout *= 2;
	}
}

static apptimer_timeout_t next_alarm;
static void set_hardware_timer_alaram_counter(apptimer_timeout_t t)
{
	next_alarm = t;
}

TEST_GROUP(AppTimer_WithHardwareTimer) {
	void setup(void) {
		nr_called = 0;
		next_alarm = 0;
		apptimer_init(set_hardware_timer_alaram_counter);
	}
	void teardown(void) {
		apptimer_deinit();
	}
};

TEST(AppTimer_WithHardwareTimer, first_timer_ShouldSetAlarm) {
	apptimer_static_t timer1;
	apptimer_static_t timer2;
	apptimer_static_t timer3;
	apptimer_create_static(&timer1, false, callback);
	apptimer_create_static(&timer2, false, callback);
	apptimer_create_static(&timer3, false, callback);
	apptimer_start(&timer1, 18, NULL);
	apptimer_start(&timer2, 9, NULL);
	apptimer_start(&timer3, 8, NULL);
	LONGS_EQUAL(8, next_alarm);
	apptimer_schedule(8);
	LONGS_EQUAL(1, next_alarm);
	apptimer_schedule(1);
	LONGS_EQUAL(8, next_alarm);
	apptimer_schedule(10);
	LONGS_EQUAL(0, apptimer_count());
}
