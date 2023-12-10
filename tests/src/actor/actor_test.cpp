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

#include "libmcu/actor.h"
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

void actor_timer_boot(void) {
}

static void actor_handler(struct actor *self, struct actor_msg *msg) {
	mock().actualCall(__func__)
		.withParameter("self", self)
		.withParameter("msg", msg);

	if (msg) {
		actor_free(msg);
	}

	sem_post(&done);
}

TEST_GROUP(ACTOR) {
	uint8_t msgbuf[1024];

	void setup(void) {
		pthread_mutex_init(&lock, NULL);
		sem_init(&done, 0, 0);

		actor_init(msgbuf, sizeof(msgbuf), 4096UL);
	}
	void teardown(void) {
                /* give some time space for new threads to take place to run
                 * before killed */
                usleep(100);
                actor_deinit();
		sem_destroy(&done);

		mock().checkExpectations();
		mock().clear();
	}
};

TEST(ACTOR, cap_ShouldReturnMessageBufferSize) {
	LONGS_EQUAL(1008, actor_cap());
}

TEST(ACTOR, len_ShouldReturnAllocatedBytes) {
	LONGS_EQUAL(0, actor_len());
	struct actor_msg *p1 = actor_alloc(sizeof(*p1));
	LONGS_EQUAL(24, actor_len());
	struct actor_msg *p2 = actor_alloc(sizeof(*p2));
	LONGS_EQUAL(48, actor_len());
	actor_free(p1);
	LONGS_EQUAL(24, actor_len());
	actor_free(p2);
	LONGS_EQUAL(0, actor_len());
}

#if 0
TEST(ACTOR, queue_len_ShouldReturnNumberOfMessagesInTheQueue) {
	LONGS_EQUAL(0, actor_queue_len(&queue));

	struct actor actor1;
	struct actor_msg *msg1 = actor_alloc(sizeof(*msg1));
	struct actor_msg *msg2 = actor_alloc(sizeof(*msg2));
	actor_set(&actor1, actor_handler, 0, &queue);
	actor_send(&actor1, msg1);
	LONGS_EQUAL(1, actor_queue_len(&queue));
	actor_send(&actor1, msg2);
	LONGS_EQUAL(2, actor_queue_len(&queue));
}

TEST(ACTOR, actor_init_ShouldRegisterActorInTheQueueOnlyOnce) {
}
#endif

TEST(ACTOR, send_ShouldIgnoreDuplicatedMessage) {
	struct actor actor1;
	struct actor_msg *msg1 = actor_alloc(sizeof(*msg1));
	actor_set(&actor1, actor_handler, 0);

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor1)
		.withParameter("msg", msg1);

	actor_send(&actor1, msg1);
	actor_send(&actor1, msg1);

	sem_wait(&done);
}

TEST(ACTOR, send_ShouldDispatchHandlers) {
	struct actor actor1;
	struct actor actor2;
	struct actor_msg *msg1 = actor_alloc(sizeof(*msg1));
	struct actor_msg *msg2 = actor_alloc(sizeof(*msg2));
	actor_set(&actor1, actor_handler, 0);
	actor_set(&actor2, actor_handler, 0);

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor1)
		.withParameter("msg", msg1);
	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor2)
		.withParameter("msg", msg2);

	actor_send(&actor1, msg1);
	actor_send(&actor2, msg2);

	sem_wait(&done);
	sem_wait(&done);
}

TEST(ACTOR, send_ShouldDispatchHandler_WhenNullMessageGiven) {
	struct actor actor1;
	actor_set(&actor1, actor_handler, 0);

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor1)
		.withParameter("msg", (struct actor_msg *)NULL);

	actor_send(&actor1, NULL);
	sem_wait(&done);
}
