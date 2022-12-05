/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include <semaphore.h>
#include "libmcu/ao.h"

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
	struct ao *ao;

	void setup(void) {
		sem_init(&done, 0, 0);
		ao = ao_create(4096, 0);
	}
	void teardown(void) {
		ao_destroy(ao);

		mock().checkExpectations();
		mock().clear();
	}
};

TEST(AO, start_ShouldDispatchNullEvent) {
	mock().expectOneCall("dispatch").withParameter("event", (const struct ao_event *)0);

	ao_start(ao, dispatch);
	sem_wait(&done);
	ao_stop(ao);
}

TEST(AO, post_ShouldDispatchTheEventGiven) {
	struct ao_event evt = { .type = 1234 };

	mock().expectOneCall("dispatch").withParameter("event", (const struct ao_event *)0);
	mock().expectOneCall("dispatch").withParameter("event", (const struct ao_event *)&evt);

	ao_start(ao, dispatch);
	sem_wait(&done);
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
