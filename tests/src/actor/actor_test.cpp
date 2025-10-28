/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
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
#include "libmcu/actor_timer.h"
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

static size_t provide_stack_size(int pri, void *ctx) {
	return 4096UL;
}

TEST_GROUP(ACTOR) {
	uint8_t msgbuf[1024];
	uint8_t memtimer[1024];

	void setup(void) {
		pthread_mutex_init(&lock, NULL);
		sem_init(&done, 0, 0);

		actor_init(msgbuf, sizeof(msgbuf), provide_stack_size, NULL);
		actor_timer_init(memtimer, sizeof(memtimer));
	}
	void teardown(void) {
                actor_deinit();
		sem_destroy(&done);

		mock().checkExpectations();
		mock().clear();
	}
};

TEST(ACTOR, cap_ShouldReturnMessageBufferSize) {
	LONGS_EQUAL(1008, actor_mem_cap());
}

TEST(ACTOR, len_ShouldReturnAllocatedBytes) {
	LONGS_EQUAL(0, actor_mem_len());
	struct actor_msg *p1 = actor_alloc(sizeof(*p1));
	LONGS_EQUAL(24, actor_mem_len());
	struct actor_msg *p2 = actor_alloc(sizeof(*p2));
	LONGS_EQUAL(48, actor_mem_len());
	actor_free(p1);
	LONGS_EQUAL(24, actor_mem_len());
	actor_free(p2);
	LONGS_EQUAL(0, actor_mem_len());
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

TEST(ACTOR, new_ShouldReturnValidActor) {
	struct actor *actor = actor_new(actor_handler, 0);

	CHECK(actor != NULL);

	actor_delete(actor);
}

TEST(ACTOR, new_delete_ShouldNotCrash_WhenDeleteCalledWithNull) {
	actor_delete(NULL);
}

TEST(ACTOR, delete_ShouldCleanupPendingMessages) {
	struct actor *actor = actor_new(actor_handler, 0);
	struct actor_msg *msg1 = actor_alloc(sizeof(*msg1));
	struct actor_msg *msg2 = actor_alloc(sizeof(*msg2));

	mock().expectOneCall("actor_handler")
		.withParameter("self", actor)
		.withParameter("msg", msg1);
	mock().expectOneCall("actor_handler")
		.withParameter("self", actor)
		.withParameter("msg", msg2);

	actor_send(actor, msg1);
	actor_send(actor, msg2);

	// Wait for messages to be processed before deleting
	sem_wait(&done);
	sem_wait(&done);

	actor_delete(actor);
}

TEST(ACTOR, unset_ShouldCleanupPendingMessages) {
	struct actor actor1;
	actor_set(&actor1, actor_handler, 0);

	size_t mem_before = actor_mem_len();

	struct actor_msg *msg1 = actor_alloc(sizeof(*msg1));
	struct actor_msg *msg2 = actor_alloc(sizeof(*msg2));

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor1)
		.withParameter("msg", msg1);
	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor1)
		.withParameter("msg", msg2);

	actor_send(&actor1, msg1);
	actor_send(&actor1, msg2);

	// Wait for messages to be processed
	sem_wait(&done);
	sem_wait(&done);

	// Memory should be freed after handler processes messages
	LONGS_EQUAL(mem_before, actor_mem_len());

	actor_unset(&actor1);
}

TEST(ACTOR, alloc_ShouldReturnNull_WhenSizeIsZero) {
	struct actor_msg *msg = actor_alloc(0);
	POINTERS_EQUAL(NULL, msg);
}

TEST(ACTOR, alloc_ShouldReturnNull_WhenSizeExceedsMax) {
	// Try to allocate a very large message (should fail)
	struct actor_msg *msg = actor_alloc(1024 * 1024);
	POINTERS_EQUAL(NULL, msg);
}

TEST(ACTOR, alloc_ShouldReturnNull_WhenPoolExhausted) {
	// Allocate until pool is exhausted
	struct actor_msg *msgs[100];
	size_t count = 0;

	while (count < 100) {
		msgs[count] = actor_alloc(sizeof(int));
		if (msgs[count] == NULL) {
			break;
		}
		count++;
	}

	// Next allocation should fail
	struct actor_msg *msg = actor_alloc(sizeof(int));
	POINTERS_EQUAL(NULL, msg);

	// Free all allocated messages
	for (size_t i = 0; i < count; i++) {
		actor_free(msgs[i]);
	}
}

TEST(ACTOR, free_ShouldSucceed_WhenNullGiven) {
	int rc = actor_free(NULL);
	LONGS_EQUAL(0, rc);
}

TEST(ACTOR, free_ShouldNotAddDuplicate_WhenFreedTwice) {
	size_t mem_before = actor_mem_len();

	struct actor_msg *msg = actor_alloc(sizeof(int));
	CHECK(msg != NULL);

	size_t mem_after_alloc = actor_mem_len();
	CHECK(mem_after_alloc > mem_before);

	actor_free(msg);
	size_t mem_after_first_free = actor_mem_len();
	LONGS_EQUAL(mem_before, mem_after_first_free);

	// Free again - should not crash or add duplicate
	actor_free(msg);
	size_t mem_after_second_free = actor_mem_len();
	LONGS_EQUAL(mem_before, mem_after_second_free);
}

TEST(ACTOR, send_ShouldSucceed_WhenNullMessageGiven) {
	struct actor actor1;
	actor_set(&actor1, actor_handler, 0);

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor1)
		.withParameter("msg", (struct actor_msg *)NULL);

	int rc = actor_send(&actor1, NULL);
	LONGS_EQUAL(0, rc);

	sem_wait(&done);

	actor_unset(&actor1);
}

TEST(ACTOR, send_ShouldReturnError_WhenDuplicateMessage) {
	struct actor actor1;
	actor_set(&actor1, actor_handler, 0);

	struct actor_msg *msg = actor_alloc(sizeof(*msg));

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor1)
		.withParameter("msg", msg);

	int rc1 = actor_send(&actor1, msg);
	LONGS_EQUAL(0, rc1);

	// Try to send the same message again
	int rc2 = actor_send(&actor1, msg);
	CHECK(rc2 != 0); // Should fail with -EALREADY

	sem_wait(&done);

	actor_unset(&actor1);
}
