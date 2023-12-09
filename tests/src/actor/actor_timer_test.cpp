/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#include "libmcu/actor_timer.h"
#include "libmcu/actor_overrides.h"
#include "libmcu/assert.h"

static pthread_mutex_t lock;
static sem_t done;

struct actor_msg {
	int id;
};

void libmcu_assertion_failed(const uintptr_t *pc, const uintptr_t *lr) {
	mock().actualCall(__func__);
}

void actor_lock(void) {
	pthread_mutex_lock(&lock);
}

void actor_unlock(void) {
	pthread_mutex_unlock(&lock);
}

static void actor_handler(struct actor *self, struct actor_msg *msg) {
	mock().actualCall(__func__)
		.withParameter("self", self)
		.withParameter("msg", msg);

	actor_free(msg);
	sem_post(&done);
}

TEST_GROUP(ACTOR_TIMER) {
	uint8_t memmsg[1024];
	uint8_t memtimer[1024];

	void setup(void) {
		pthread_mutex_init(&lock, NULL);
		sem_init(&done, 0, 0);

		actor_init(memmsg, sizeof(memmsg), 4096UL);
		actor_timer_init(memtimer, sizeof(memtimer));
	}
	void teardown(void) {
                usleep(50);
                actor_deinit();
		sem_destroy(&done);

		mock().checkExpectations();
		mock().clear();
	}
};

TEST(ACTOR_TIMER, start_ShouldSendActor_WhenTimedout) {
	struct actor actor;
	struct actor_msg *msg = actor_alloc(sizeof(*msg));
	actor_set(&actor, actor_handler, 0);

	uint32_t defer_ms = 1000;
	struct actor_timer *timer = actor_timer_new(&actor, msg, defer_ms, false);
	actor_timer_start(timer);

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg);

	actor_timer_step(defer_ms);
	sem_wait(&done);

	LONGS_EQUAL(0, actor_timer_len());
}

TEST(ACTOR_TIMER, start_ShouldSendActorRepeatly_WhenIntervalGiven) {
	struct actor actor;
	struct actor_msg *msg = actor_alloc(sizeof(*msg));
	actor_set(&actor, actor_handler, 0);

	uint32_t defer_ms = 1000;
	struct actor_timer *timer = actor_timer_new(&actor, msg, defer_ms, true);
	actor_timer_start(timer);

	for (int i = 0; i < 10; i++) {
		mock().expectOneCall("actor_handler")
			.withParameter("self", &actor)
			.withParameter("msg", msg);
		actor_timer_step(defer_ms);
		sem_wait(&done);
	}

	actor_timer_stop(timer);
	LONGS_EQUAL(1, actor_timer_len());
	actor_timer_delete(timer);
	LONGS_EQUAL(0, actor_timer_len());

	actor_free(msg);
}

TEST(ACTOR_TIMER, new_ShouldReturnNull_WhenAllocationFailed) {
	for (size_t i = 0; i < actor_timer_cap(); i++) {
		CHECK(actor_timer_new(0, 0, 10, 0) != NULL);
	}
	CHECK(actor_timer_new(0, 0, 10, 0) == NULL);
	LONGS_EQUAL(actor_timer_cap(), actor_timer_len());
}
