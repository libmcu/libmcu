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

TEST_GROUP(ACTOR_TIMER) {
	uint8_t memtimer[1024];

	void setup(void) {
		pthread_mutex_init(&lock, NULL);

		actor_timer_init(memtimer, sizeof(memtimer));
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(ACTOR_TIMER, t) {
}
