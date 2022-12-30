/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include <semaphore.h>
#include "libmcu/ao.h"
#include "libmcu/ao_timer.h"

struct my_ao {
	struct ao base;
	int my_own_data;
};

struct ao_event {
	int type;
};

static sem_t done;

static void dispatch(struct ao * const ao, const struct ao_event * const event)
{
	mock().actualCall(__func__).withParameter("event", event);
	sem_post(&done);
}

TEST_GROUP(AO) {
	struct my_ao my_ao;
	struct ao *ao;

	void setup(void) {
		sem_init(&done, 0, 0);
		ao_timer_reset();
		ao = ao_create(&my_ao.base, 4096, 0);
	}
	void teardown(void) {
		ao_destroy(ao);

		mock().checkExpectations();
		mock().clear();
	}
};

TEST(AO, start_ShouldDispatchNothing) {
	ao_start(ao, dispatch);
	ao_stop(ao);
}

TEST(AO, post_ShouldDispatchTheEventGiven) {
	struct ao_event evt = { .type = 1234 };

	mock().expectOneCall("dispatch").withParameter("event", (const struct ao_event *)&evt);

	ao_start(ao, dispatch);
	ao_post(ao, &evt);
	sem_wait(&done);
	ao_stop(ao);
}

TEST(AO, post_ShouldReturnENOSPC_WhenQueueIsFull) {
	for (unsigned int i = 0; i < AO_EVENT_MAXLEN; i++) {
		LONGS_EQUAL(0, ao_post(ao, 0));
	}
	LONGS_EQUAL(-ENOSPC, ao_post(ao, 0));
}

TEST(AO, post_ShouldDispatchEvents_WhenSpcificEventsGiven) {
	struct ao_event evt[AO_EVENT_MAXLEN];

	for (unsigned int i = 0; i < AO_EVENT_MAXLEN; i++) {
		ao_post(ao, &evt[i]);
	}
	for (unsigned int i = 0; i < AO_EVENT_MAXLEN; i++) {
		mock().expectOneCall("dispatch").withParameter("event", (const struct ao_event *)&evt[i]);
	}

	ao_start(ao, dispatch);
	for (unsigned int i = 0; i < AO_EVENT_MAXLEN; i++) {
		sem_wait(&done);
	}
	ao_stop(ao);
}

TEST(AO, post_defer_ShouldPostAfterTimeout_WhenTimeoutGiven) {
	struct ao_event evt = { .type = 1 };
	uint32_t timeout_ms = 10;
	ao_post_defer(ao, &evt, timeout_ms);

	mock().expectOneCall("dispatch").withParameter("event", (const struct ao_event *)&evt);
	ao_start(ao, dispatch);
	ao_timer_step(timeout_ms);
	sem_wait(&done);
	ao_stop(ao);
}

TEST(AO, post_defer_ShouldPostRepeatly_WhenIntervalGiven) {
	struct ao_event evt = { .type = 1 };
	uint32_t timeout_ms = 10;

	ao_start(ao, dispatch);

	ao_post_repeat(ao, &evt, timeout_ms, timeout_ms);
	for (int i = 0; i < 10; i++) {
		mock().expectOneCall("dispatch").withParameter("event", (const struct ao_event *)&evt);
		ao_timer_step(timeout_ms);
		sem_wait(&done);
	}

	ao_stop(ao);
}

TEST(AO, post_defer_ShouldReturnENOSPC_WhenAllocationFailed) {
	struct ao_event evt = { .type = 1 };
	uint32_t timeout_ms = 10;

	for (unsigned int i = 0; i < AO_TIMER_MAXLEN; i++) {
		LONGS_EQUAL(0, ao_post_defer(ao, &evt, timeout_ms));
	}
	LONGS_EQUAL(-ENOSPC, ao_post_defer(ao, &evt, timeout_ms));
}

TEST(AO, step_ShouldKeepEventsAndTryPostAgainInTheNextStep_WhenPostFailed) {
	struct ao_event evt = { .type = 1 };
	uint32_t timeout_ms = 10;

	for (unsigned int i = 0; i < AO_EVENT_MAXLEN; i++) {
		mock().expectOneCall("dispatch").withParameter("event", (const struct ao_event *)0);
		ao_post(ao, 0);
	}

	mock().expectOneCall("dispatch").withParameter("event", (const struct ao_event *)&evt);
	ao_post_defer(ao, &evt, timeout_ms);
	ao_timer_step(timeout_ms);

	ao_start(ao, dispatch);
	for (unsigned int i = 0; i < AO_EVENT_MAXLEN; i++) {
		sem_wait(&done);
	}
	ao_timer_step(1);
	sem_wait(&done);
	ao_stop(ao);
}

TEST(AO, cancel_ShouldCancelTimersInQueue_WhenEventsGiven) {
	struct ao_event evt1 = { 0, };
	struct ao_event evt2;
	struct ao_event evt3;

	ao_post_defer(ao, &evt1, 1000);
	ao_post_defer(ao, &evt2, 1000);
	ao_post_defer(ao, &evt3, 1000);

	LONGS_EQUAL(1, ao_cancel(ao, &evt1));
	LONGS_EQUAL(1, ao_cancel(ao, &evt2));
	LONGS_EQUAL(1, ao_cancel(ao, &evt3));
}

TEST(AO, cancel_ShouldCancelTimersInQueue_WhenEventGiven) {
	struct ao_event evt = { 0, };

	ao_post_defer(ao, &evt, 1000);
	ao_post_defer(ao, &evt, 1000);
	ao_post_defer(ao, &evt, 1000);

	LONGS_EQUAL(3, ao_cancel(ao, &evt));
}

TEST(AO, post_if_unique_ShouldDispatchTheEvent_WhenNotTheSameEventQueuedAndArmed) {
	struct ao_event evt = { 0, };

	mock().expectOneCall("dispatch")
		.withParameter("event", (const struct ao_event *)&evt);

	ao_start(ao, dispatch);
	LONGS_EQUAL(0, ao_post_if_unique(ao, &evt));
	sem_wait(&done);
	ao_stop(ao);
}

TEST(AO, post_if_unique_ShouldReturnEEXIT_WhenTheSameEventIsAlreadyInTheQueue) {
	struct ao_event evt = { 0, };
	ao_post_if_unique(ao, &evt);
	LONGS_EQUAL(-EEXIST, ao_post_if_unique(ao, &evt));
}

TEST(AO, post_if_unique_ShouldReturnEEXIT_WhenTheSameEventIsArmedAlready) {
	struct ao_event evt = { 0, };
	ao_post_defer(ao, &evt, 1000);
	LONGS_EQUAL(-EEXIST, ao_post_if_unique(ao, &evt));
}

TEST(AO, post_defer_if_unique_ShouldPostAfterTimeout_WhenTimeoutGiven) {
	struct ao_event evt = { 0, };
	uint32_t timeout_ms = 10;
	ao_post_defer_if_unique(ao, &evt, timeout_ms);

	mock().expectOneCall("dispatch")
		.withParameter("event", (const struct ao_event *)&evt);

	ao_start(ao, dispatch);
	ao_timer_step(timeout_ms);
	sem_wait(&done);
	ao_stop(ao);
}

TEST(AO, post_defer_if_unique_ShouldReturnEEXIST_WhenTheSameEventIsAlreadyIn) {
	struct ao_event evt = { 0, };
	uint32_t timeout_ms = 10;

	ao_post(ao, &evt);
	LONGS_EQUAL(-EEXIST, ao_post_defer_if_unique(ao, &evt, timeout_ms));
}

TEST(AO, post_defer_if_unique_ShouldReturnEEXIST_WhenTheSameEventArmedAlready) {
	struct ao_event evt = { 0, };
	uint32_t timeout_ms = 10;

	ao_post_defer(ao, &evt, timeout_ms);
	LONGS_EQUAL(-EEXIST, ao_post_defer_if_unique(ao, &evt, timeout_ms));
}

TEST(AO, post_repeat_if_unique_ShouldPostRepeatly) {
	struct ao_event evt = { 0, };
	uint32_t timeout_ms = 10;

	ao_start(ao, dispatch);

	ao_post_repeat_if_unique(ao, &evt, timeout_ms, timeout_ms);
	for (int i = 0; i < 10; i++) {
		mock().expectOneCall("dispatch")
			.withParameter("event", (const struct ao_event *)&evt);
		ao_timer_step(timeout_ms);
		sem_wait(&done);
	}

	ao_stop(ao);
}

TEST(AO, post_repeat_if_unique_ShouldReturnEEXIST_WhenTheSameEventIsAlreadyIn) {
	struct ao_event evt = { 0, };
	uint32_t timeout_ms = 10;

	ao_post(ao, &evt);
	LONGS_EQUAL(-EEXIST, ao_post_repeat_if_unique(ao, &evt,
				timeout_ms, timeout_ms));
}

TEST(AO, post_repeat_if_unique_ShouldReturnEEXIST_WhenTheSameEventArmedAlready) {
	struct ao_event evt = { 0, };
	uint32_t timeout_ms = 10;

	ao_post_repeat(ao, &evt, timeout_ms, timeout_ms);
	LONGS_EQUAL(-EEXIST, ao_post_repeat_if_unique(ao, &evt,
				timeout_ms, timeout_ms));
}
