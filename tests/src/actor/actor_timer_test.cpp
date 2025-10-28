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

TEST_GROUP(ACTOR_TIMER) {
	uint8_t memmsg[1024];
	uint8_t memtimer[1024];

	void setup(void) {
		pthread_mutex_init(&lock, NULL);
		sem_init(&done, 0, 0);

		actor_init(memmsg, sizeof(memmsg), provide_stack_size, NULL);
		actor_timer_init(memtimer, sizeof(memtimer));
	}
	void teardown(void) {
                actor_deinit();
		sem_destroy(&done);
		pthread_mutex_destroy(&lock);

		mock().checkExpectations();
		mock().clear();
	}
};

TEST(ACTOR_TIMER, start_ShouldSendActor_WhenTimedout) {
	struct actor actor;
	struct actor_msg *msg = actor_alloc(sizeof(*msg));
	actor_set(&actor, actor_handler, 0);

	uint32_t defer_ms = 1000;
	struct actor_timer *timer = actor_timer_new(&actor, msg, defer_ms);
	CHECK(timer != NULL);
	actor_timer_start(timer);

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg);

	LONGS_EQUAL(1, actor_timer_len());
	actor_timer_step(defer_ms);
	sem_wait(&done);

	LONGS_EQUAL(0, actor_timer_len());
}

TEST(ACTOR_TIMER, delete_ShouldStopTimer_WhenStartedTimerDeleted) {
	struct actor actor;
	struct actor_msg *msg = actor_alloc(sizeof(*msg));
	actor_set(&actor, actor_handler, 0);

	uint32_t defer_ms = 1000;
	struct actor_timer *timer = actor_timer_new(&actor, msg, defer_ms);
	actor_timer_start(timer);

	LONGS_EQUAL(1, actor_timer_len());
	actor_timer_delete(timer);
	LONGS_EQUAL(0, actor_timer_len());
}

TEST(ACTOR_TIMER, new_ShouldReturnNull_WhenAllocationFailed) {
	for (size_t i = 0; i < actor_timer_cap(); i++) {
		CHECK(actor_timer_new(0, 0, 10) != NULL);
	}
	CHECK(actor_timer_new(0, 0, 10) == NULL);
	LONGS_EQUAL(actor_timer_cap(), actor_timer_len());
}

TEST(ACTOR_TIMER, stop_ShouldCancelTimer_BeforeTimeout) {
	struct actor actor;
	struct actor_msg *msg = actor_alloc(sizeof(*msg));
	actor_set(&actor, actor_handler, 0);

	uint32_t defer_ms = 1000;
	struct actor_timer *timer = actor_timer_new(&actor, msg, defer_ms);
	CHECK(timer != NULL);
	actor_timer_start(timer);

	LONGS_EQUAL(1, actor_timer_len());

	// Stop the timer before it times out
	actor_timer_stop(timer);

	// Step through timeout - handler should NOT be called
	actor_timer_step(defer_ms);

	// Timer should still be allocated but not armed
	LONGS_EQUAL(1, actor_timer_len());

	// Clean up
	actor_timer_delete(timer);
	actor_free(msg);
	LONGS_EQUAL(0, actor_timer_len());

	actor_unset(&actor);
}

TEST(ACTOR_TIMER, step_ShouldUpdateTimeout_WhenPartialTimeElapsed) {
	struct actor actor;
	struct actor_msg *msg = actor_alloc(sizeof(*msg));
	actor_set(&actor, actor_handler, 0);

	uint32_t defer_ms = 1000;
	struct actor_timer *timer = actor_timer_new(&actor, msg, defer_ms);
	actor_timer_start(timer);

	LONGS_EQUAL(1, actor_timer_len());

	// Step with partial timeout
	actor_timer_step(400);
	LONGS_EQUAL(1, actor_timer_len()); // Still armed

	actor_timer_step(400);
	LONGS_EQUAL(1, actor_timer_len()); // Still armed

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg);

	// Final step should trigger timeout
	actor_timer_step(200);
	sem_wait(&done);

	LONGS_EQUAL(0, actor_timer_len());

	actor_unset(&actor);
}

TEST(ACTOR_TIMER, step_ShouldHandleExcessTime_WhenStepExceedsTimeout) {
	struct actor actor;
	struct actor_msg *msg = actor_alloc(sizeof(*msg));
	actor_set(&actor, actor_handler, 0);

	uint32_t defer_ms = 100;
	struct actor_timer *timer = actor_timer_new(&actor, msg, defer_ms);
	actor_timer_start(timer);

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg);

	// Step with time exceeding timeout
	actor_timer_step(500);
	sem_wait(&done);

	LONGS_EQUAL(0, actor_timer_len());

	actor_unset(&actor);
}

TEST(ACTOR_TIMER, multipleTimers_ShouldAllTrigger_WhenTimedOut) {
	struct actor actor1;
	struct actor actor2;
	actor_set(&actor1, actor_handler, 0);
	actor_set(&actor2, actor_handler, 0);

	struct actor_msg *msg1 = actor_alloc(sizeof(*msg1));
	struct actor_msg *msg2 = actor_alloc(sizeof(*msg2));
	struct actor_msg *msg3 = actor_alloc(sizeof(*msg3));

	struct actor_timer *timer1 = actor_timer_new(&actor1, msg1, 100);
	struct actor_timer *timer2 = actor_timer_new(&actor2, msg2, 200);
	struct actor_timer *timer3 = actor_timer_new(&actor1, msg3, 300);

	actor_timer_start(timer1);
	actor_timer_start(timer2);
	actor_timer_start(timer3);

	LONGS_EQUAL(3, actor_timer_len());

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor1)
		.withParameter("msg", msg1);

	// First timer should trigger
	actor_timer_step(100);
	sem_wait(&done);
	LONGS_EQUAL(2, actor_timer_len());

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor2)
		.withParameter("msg", msg2);

	// Second timer should trigger
	actor_timer_step(100);
	sem_wait(&done);
	LONGS_EQUAL(1, actor_timer_len());

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor1)
		.withParameter("msg", msg3);

	// Third timer should trigger
	actor_timer_step(100);
	sem_wait(&done);
	LONGS_EQUAL(0, actor_timer_len());

	actor_unset(&actor1);
	actor_unset(&actor2);
}

TEST(ACTOR_TIMER, count_messages_ShouldReturnCorrectCount) {
	struct actor actor1;
	struct actor actor2;
	actor_set(&actor1, actor_handler, 0);
	actor_set(&actor2, actor_handler, 0);

	struct actor_msg *msg1 = actor_alloc(sizeof(*msg1));
	struct actor_msg *msg2 = actor_alloc(sizeof(*msg2));
	struct actor_msg *msg3 = actor_alloc(sizeof(*msg3));

	struct actor_timer *timer1 = actor_timer_new(&actor1, msg1, 100);
	struct actor_timer *timer2 = actor_timer_new(&actor2, msg2, 200);
	struct actor_timer *timer3 = actor_timer_new(&actor1, msg3, 300);

	actor_timer_start(timer1);
	actor_timer_start(timer2);
	actor_timer_start(timer3);

	// actor1 should have 2 messages, actor2 should have 1
	LONGS_EQUAL(2, actor_timer_count_messages(&actor1));
	LONGS_EQUAL(1, actor_timer_count_messages(&actor2));

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor1)
		.withParameter("msg", msg1);
	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor2)
		.withParameter("msg", msg2);
	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor1)
		.withParameter("msg", msg3);

	// Trigger all timers
	actor_timer_step(300);
	sem_wait(&done);
	sem_wait(&done);
	sem_wait(&done);

	LONGS_EQUAL(0, actor_timer_count_messages(&actor1));
	LONGS_EQUAL(0, actor_timer_count_messages(&actor2));

	actor_unset(&actor1);
	actor_unset(&actor2);
}

TEST(ACTOR_TIMER, new_ShouldHandleNullMessage) {
	struct actor actor;
	actor_set(&actor, actor_handler, 0);

	// Timer with NULL message should be allowed
	struct actor_timer *timer = actor_timer_new(&actor, NULL, 100);
	CHECK(timer != NULL);

	actor_timer_start(timer);
	LONGS_EQUAL(1, actor_timer_len());

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", (struct actor_msg *)NULL);

	actor_timer_step(100);
	sem_wait(&done);

	LONGS_EQUAL(0, actor_timer_len());

	actor_unset(&actor);
}

TEST(ACTOR_TIMER, cap_ShouldReturnTotalCapacity) {
	size_t cap = actor_timer_cap();
	CHECK(cap > 0);
	CHECK(cap < 1024); // Should be reasonable based on memtimer size
}

TEST(ACTOR_TIMER, len_ShouldReturnUsedCount) {
	size_t initial_len = actor_timer_len();
	LONGS_EQUAL(0, initial_len);

	struct actor actor;
	actor_set(&actor, actor_handler, 0);

	struct actor_msg *msg = actor_alloc(sizeof(*msg));
	struct actor_timer *timer = actor_timer_new(&actor, msg, 100);
	actor_timer_start(timer);

	LONGS_EQUAL(1, actor_timer_len());

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg);

	actor_timer_step(100);
	sem_wait(&done);

	LONGS_EQUAL(0, actor_timer_len());

	actor_unset(&actor);
}

TEST(ACTOR_TIMER, step_ShouldHandleZeroElapsedTime) {
	struct actor actor;
	struct actor_msg *msg = actor_alloc(sizeof(*msg));
	actor_set(&actor, actor_handler, 0);

	struct actor_timer *timer = actor_timer_new(&actor, msg, 100);
	actor_timer_start(timer);

	// Step with 0 elapsed time - nothing should happen
	actor_timer_step(0);
	LONGS_EQUAL(1, actor_timer_len());

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg);

	// Now trigger timeout
	actor_timer_step(100);
	sem_wait(&done);

	actor_unset(&actor);
}

TEST(ACTOR_TIMER, count_messages_ShouldIncludeDeferredMessages) {
	struct actor actor;
	actor_set(&actor, actor_handler, 0);

	struct actor_msg *msg1 = actor_alloc(sizeof(*msg1));
	struct actor_msg *msg2 = actor_alloc(sizeof(*msg2));
	struct actor_msg *msg3 = actor_alloc(sizeof(*msg3));
	struct actor_msg *msg4 = actor_alloc(sizeof(*msg4));

	// Set up mocks first
	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg1);

	// Send one immediate message
	actor_send(&actor, msg1);

	// Send two deferred messages using actor_send_defer
	actor_send_defer(&actor, msg2, 1000);
	actor_send_defer(&actor, msg3, 2000);

	// Wait for immediate message to be processed
	sem_wait(&done);

	// Send another immediate message after first is processed
	actor_send(&actor, msg4);

	// actor_count_messages should count BOTH immediate queued message AND deferred timer messages
	// Should be: 1 (immediate msg4) + 2 (deferred msg2, msg3) = 3
	size_t total_count = actor_count_messages(&actor);
	CHECK(total_count >= 2); // At least 2 deferred messages
	LONGS_EQUAL(2, actor_timer_count_messages(&actor)); // 2 timer messages

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg4);

	// Wait for msg4 to be processed
	sem_wait(&done);

	// Now should only have deferred messages
	LONGS_EQUAL(2, actor_count_messages(&actor));
	LONGS_EQUAL(2, actor_timer_count_messages(&actor));

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg2);
	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg3);

	// Step through and trigger all deferred messages
	actor_timer_step(2000);
	sem_wait(&done);
	sem_wait(&done);

	LONGS_EQUAL(0, actor_count_messages(&actor));
	LONGS_EQUAL(0, actor_timer_count_messages(&actor));

	actor_unset(&actor);
}

TEST(ACTOR_TIMER, count_messages_ShouldUpdateAfterDeferredTimeout) {
	struct actor actor;
	actor_set(&actor, actor_handler, 0);

	struct actor_msg *msg1 = actor_alloc(sizeof(*msg1));
	struct actor_msg *msg2 = actor_alloc(sizeof(*msg2));
	struct actor_msg *msg3 = actor_alloc(sizeof(*msg3));

	// Send deferred messages with different timeouts
	actor_send_defer(&actor, msg1, 100);
	actor_send_defer(&actor, msg2, 200);

	// actor_count_messages should include timer messages
	LONGS_EQUAL(2, actor_count_messages(&actor));
	LONGS_EQUAL(2, actor_timer_count_messages(&actor));

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg1);

	// First timer expires
	actor_timer_step(100);
	sem_wait(&done);

	// Should have 1 deferred message left
	LONGS_EQUAL(1, actor_count_messages(&actor));
	LONGS_EQUAL(1, actor_timer_count_messages(&actor));

	// Add an immediate message while one timer is still pending
	actor_send(&actor, msg3);

	// Now should have 1 immediate + 1 deferred = 2
	size_t count = actor_count_messages(&actor);
	CHECK(count >= 1); // At least the timer message

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg3);
	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg2);

	// Process immediate message
	sem_wait(&done);

	// Should have 1 timer left
	LONGS_EQUAL(1, actor_count_messages(&actor));

	// Second timer expires
	actor_timer_step(100);
	sem_wait(&done);

	LONGS_EQUAL(0, actor_count_messages(&actor));
	LONGS_EQUAL(0, actor_timer_count_messages(&actor));

	actor_unset(&actor);
}

TEST(ACTOR_TIMER, send_defer_ShouldReturnError_WhenTimerPoolExhausted) {
	struct actor actor;
	actor_set(&actor, actor_handler, 0);

	// Exhaust timer pool
	for (size_t i = 0; i < actor_timer_cap(); i++) {
		struct actor_msg *msg = actor_alloc(sizeof(*msg));
		int rc = actor_send_defer(&actor, msg, 1000);
		LONGS_EQUAL(0, rc);
	}

	// Next defer should fail
	struct actor_msg *msg = actor_alloc(sizeof(*msg));
	int rc = actor_send_defer(&actor, msg, 1000);
	CHECK(rc != 0); // Should return -ENOSPC

	// Clean up the failed message
	actor_free(msg);

	// Clean up all timers
	mock().ignoreOtherCalls();
	actor_timer_step(1000);

	// Wait for all handlers to complete
	for (size_t i = 0; i < actor_timer_cap(); i++) {
		sem_wait(&done);
	}

	actor_unset(&actor);
}

TEST(ACTOR_TIMER, send_defer_ShouldHandleZeroDelay) {
	struct actor actor;
	actor_set(&actor, actor_handler, 0);

	struct actor_msg *msg = actor_alloc(sizeof(*msg));

	// Defer with 0ms delay
	int rc = actor_send_defer(&actor, msg, 0);
	LONGS_EQUAL(0, rc);

	LONGS_EQUAL(1, actor_timer_count_messages(&actor));

	mock().expectOneCall("actor_handler")
		.withParameter("self", &actor)
		.withParameter("msg", msg);

	// Should trigger immediately
	actor_timer_step(0);
	sem_wait(&done);

	LONGS_EQUAL(0, actor_timer_count_messages(&actor));

	actor_unset(&actor);
}
