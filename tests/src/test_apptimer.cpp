#include "CppUTest/TestHarness.h"

#include "libmcu/apptimer.h"
#include "apptimer_spy.h"

TEST_GROUP(AppTimer) {
	void setup(void) {
		apptimer_init();
	}
	void teardown(void) {
		apptimer_deinit();
	}
};

TEST(AppTimer, create_static_ShouldReturnError_WhenInvalidParamGiven) {
	LONGS_EQUAL(APPTIMER_INVALID_PARAM,
			apptimer_create_static(NULL, 0, false, NULL));
}

TEST(AppTimer, create_static_ShouldReturnSuccess) {
	apptimer_t timer1;
	LONGS_EQUAL(APPTIMER_SUCCESS,
			apptimer_create_static(&timer1, 0, false, NULL));
	apptimer_delete(&timer1);
}

TEST(AppTimer, create_ShouldReturnTimerAllocated) {
	uint32_t timeout_ms = 1;
	apptimer_t *timer = apptimer_create(timeout_ms, true, NULL);
	CHECK(timer);
	LONGS_EQUAL(TIMERSPY_CREATED, apptimerspy_get_state(timer));
	apptimer_delete(timer);
}

TEST(AppTimer, delete_ShouldReturnSuccess) {
	apptimer_t *timer = apptimer_create(0, true, NULL);
	LONGS_EQUAL(APPTIMER_SUCCESS, apptimer_delete(timer));
}

TEST(AppTimer, start_ShouldReturnSuccess_WhenOneTimeTimer) {
	apptimer_t *timer = apptimer_create(0, false, NULL);
	LONGS_EQUAL(APPTIMER_SUCCESS, apptimer_start(timer));
	LONGS_EQUAL(TIMERSPY_STARTED, apptimerspy_get_state(timer));
	apptimer_delete(timer);
}

TEST(AppTimer, start_ShouldReturnSuccess_WhenPeriodicTimer) {
	apptimer_t *timer = apptimer_create(0, true, NULL);
	LONGS_EQUAL(APPTIMER_SUCCESS, apptimer_start(timer));
	LONGS_EQUAL(TIMERSPY_STARTED_PERIODIC, apptimerspy_get_state(timer));
	apptimer_delete(timer);
}

TEST(AppTimer, stop_ShouldReturnSuccess) {
	apptimer_t *timer = apptimer_create(0, true, NULL);
	LONGS_EQUAL(APPTIMER_SUCCESS, apptimer_stop(timer));
	LONGS_EQUAL(TIMERSPY_STOPPED, apptimerspy_get_state(timer));
	apptimer_delete(timer);
}
