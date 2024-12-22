/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"
#include "libmcu/msgq.h"

static int f_lock(void *ctx) {
	return mock().actualCall(__func__)
		.withParameter("ctx", ctx)
		.returnIntValueOrDefault(0);
}

static int f_unlock(void *ctx) {
	return mock().actualCall(__func__)
		.withParameter("ctx", ctx)
		.returnIntValueOrDefault(0);
}

TEST_GROUP(MessageQueue) {
	struct msgq *msgq;
	void *sync_ctx;

	void setup(void) {
		msgq = msgq_create(128);
	}
	void teardown(void) {
		msgq_destroy(msgq);

		mock().checkExpectations();
		mock().clear();
	}
};

TEST(MessageQueue, create_ShouldReturnNull_WhenCapacityIsZero) {
	struct msgq *q = msgq_create(0);
	LONGS_EQUAL(NULL, q);
}

TEST(MessageQueue, create_ShouldReturnValidPointer_WhenCapacityIsGreaterThanZero) {
	CHECK_TRUE(msgq);
}

TEST(MessageQueue, set_sync_ShouldReturnZero_WhenLockAndUnlockFunctionsAreValid) {
	LONGS_EQUAL(0, msgq_set_sync(msgq, f_lock, f_unlock, sync_ctx));
}

TEST(MessageQueue, set_sync_ShouldReturnZero_WhenNullFuncationsGiven) {
	LONGS_EQUAL(0, msgq_set_sync(msgq, NULL, NULL, NULL));
}

TEST(MessageQueue, cap_ShouldReturnQueueCapacity) {
	LONGS_EQUAL(128, msgq_cap(msgq));
}

TEST(MessageQueue, cap_ShouldReturnPowerOfTwo_WhenCapacityIsNotPowerOfTwo) {
	struct msgq *q = msgq_create(150);
	LONGS_EQUAL(128, msgq_cap(q));
	msgq_destroy(q);
}

TEST(MessageQueue, len_ShouldReturnZero_WhenQueueIsEmpty) {
	LONGS_EQUAL(0, msgq_len(msgq));
}

TEST(MessageQueue, len_ShouldReturnNonZero_WhenQueueIsNotEmpty) {
	const char *msg = "hello";
	msgq_push(msgq, msg, strlen(msg));
	CHECK_TRUE(msgq_len(msgq));
}

TEST(MessageQueue, len_ShouldReturnSumOfAllMessagesAndMetadata_WhenQueueIsNotEmpty) {
	const char *msg1 = "hello";
	const char *msg2 = "world";
	msgq_push(msgq, msg1, strlen(msg1));
	msgq_push(msgq, msg2, strlen(msg2));
	LONGS_EQUAL(strlen(msg1) + strlen(msg2) + 2 * sizeof(msgq_msg_meta_t), msgq_len(msgq));
}

TEST(MessageQueue, push_ShouldCallSyncFunctions_WhenLockAndUnlockAreSet) {
	mock().expectOneCall("f_lock")
		.withParameter("ctx", sync_ctx);
	mock().expectOneCall("f_unlock")
		.withParameter("ctx", sync_ctx);
	msgq_set_sync(msgq, f_lock, f_unlock, sync_ctx);
	msgq_push(msgq, "hello", 5);
}

TEST(MessageQueue, push_ShouldReturnEAGAIN_WhenLockReturnsNonZero) {
	mock().expectOneCall("f_lock")
		.withParameter("ctx", sync_ctx)
		.andReturnValue(-1);
	msgq_set_sync(msgq, f_lock, f_unlock, sync_ctx);
	LONGS_EQUAL(-EAGAIN, msgq_push(msgq, "hello", 5));
}

TEST(MessageQueue, push_ShouldReturnUnlockError_WhenUnlockReturnsNonZero) {
	mock().expectOneCall("f_lock")
		.withParameter("ctx", sync_ctx)
		.andReturnValue(0);
	mock().expectOneCall("f_unlock")
		.withParameter("ctx", sync_ctx)
		.andReturnValue(-2);
	msgq_set_sync(msgq, f_lock, f_unlock, sync_ctx);
	LONGS_EQUAL(-2, msgq_push(msgq, "hello", 5));
}

TEST(MessageQueue, push_ShouldReturnZero_WhenSuccess) {
	LONGS_EQUAL(0, msgq_push(msgq, "hello", 5));
}

TEST(MessageQueue, push_ShouldReturnNOMEM_WhenQueueIsFull) {
	uint8_t data[128-sizeof(msgq_msg_meta_t)];
	LONGS_EQUAL(0, msgq_push(msgq, data, sizeof(data)));
	LONGS_EQUAL(-ENOMEM, msgq_push(msgq, data, 1));
}

TEST(MessageQueue, pop_ShouldReturnENOENT_WhenQueueIsEmpty) {
	uint8_t buf[16];
	LONGS_EQUAL(-ENOENT, msgq_pop(msgq, buf, sizeof(buf)));
}

TEST(MessageQueue, pop_ShouldReturnERANGE_WhenBufSizeIsSmallerThanMessageSize) {
	const char *msg = "hello";
	msgq_push(msgq, msg, strlen(msg));
	uint8_t buf[4];
	LONGS_EQUAL(-ERANGE, msgq_pop(msgq, buf, sizeof(buf)));
}

TEST(MessageQueue, pop_ShouldReturnZero_WhenSuccess) {
	const char *msg = "hello";
	msgq_push(msgq, msg, strlen(msg));
	uint8_t buf[16];
	LONGS_EQUAL((int)strlen(msg), msgq_pop(msgq, buf, sizeof(buf)));
}

TEST(MessageQueue, pop_ShouldResultInSameMessage_WhenSuccess) {
	const char *msg = "hello";
	msgq_push(msgq, msg, strlen(msg));
	uint8_t buf[16];
	LONGS_EQUAL((int)strlen(msg), msgq_pop(msgq, buf, sizeof(buf)));
	LONGS_EQUAL(0, strncmp(msg, (const char *)buf, strlen(msg)));
}

TEST(MessageQueue, pop_ShouldReturnZero_WhenSuccess_WithMultipleMessages) {
	const char *msg1 = "hello";
	const char *msg2 = "world";
	msgq_push(msgq, msg1, strlen(msg1));
	msgq_push(msgq, msg2, strlen(msg2));
	uint8_t buf[16];
	LONGS_EQUAL((int)strlen(msg1), msgq_pop(msgq, buf, sizeof(buf)));
	LONGS_EQUAL(0, strncmp(msg1, (const char *)buf, strlen(msg1)));
}

TEST(MessageQueue, pop_ShouldReturnZero_WhenSuccess_WithMultipleMessages2) {
	const char *msg1 = "hello";
	const char *msg2 = "world";
	msgq_push(msgq, msg1, strlen(msg1));
	msgq_push(msgq, msg2, strlen(msg2));
	uint8_t buf[16];
	LONGS_EQUAL((int)strlen(msg1), msgq_pop(msgq, buf, sizeof(buf)));
	LONGS_EQUAL(0, strncmp(msg1, (const char *)buf, strlen(msg1)));
	LONGS_EQUAL((int)strlen(msg2), msgq_pop(msgq, buf, sizeof(buf)));
	LONGS_EQUAL(0, strncmp(msg2, (const char *)buf, strlen(msg2)));
}

TEST(MessageQueue, pop_ShouldCallSyncFunctions_WhenLockAndUnlockAreSet) {
	mock().expectNCalls(2, "f_lock")
		.withParameter("ctx", sync_ctx);
	mock().expectNCalls(2, "f_unlock")
		.withParameter("ctx", sync_ctx);
	msgq_set_sync(msgq, f_lock, f_unlock, sync_ctx);
	msgq_push(msgq, "hello", 5);
	uint8_t buf[16];
	msgq_pop(msgq, buf, sizeof(buf));
}

TEST(MessageQueue, next_msg_size_ShouldReturnZero_WhenQueueIsEmpty) {
	LONGS_EQUAL(0, msgq_next_msg_size(msgq));
}

TEST(MessageQueue, next_msg_size_ShouldReturnMessageSize_WhenQueueIsNotEmpty) {
	const char *msg = "hello";
	msgq_push(msgq, msg, strlen(msg));
	LONGS_EQUAL(strlen(msg), msgq_next_msg_size(msgq));
}

TEST(MessageQueue, next_msg_size_ShouldReturnZero_WhenQueueIsNotEmpty) {
	const char *msg1 = "hello";
	const char *msg2 = "world";
	msgq_push(msgq, msg1, strlen(msg1));
	msgq_push(msgq, msg2, strlen(msg2));
	LONGS_EQUAL(strlen(msg1), msgq_next_msg_size(msgq));
}

TEST(MessageQueue, next_msg_ShouldCallSyncFunctions_WhenLockAndUnlockAreSet) {
	mock().expectNCalls(2, "f_lock")
		.withParameter("ctx", sync_ctx);
	mock().expectNCalls(2, "f_unlock")
		.withParameter("ctx", sync_ctx);
	msgq_set_sync(msgq, f_lock, f_unlock, sync_ctx);
	msgq_push(msgq, "hello", 5);
	msgq_next_msg_size(msgq);
}

TEST(MessageQueue, available_ShouldReturnZero_WhenQueueIsFull) {
	uint8_t data[128-sizeof(msgq_msg_meta_t)];
	LONGS_EQUAL(sizeof(data), msgq_available(msgq));
	msgq_push(msgq, data, sizeof(data));
	LONGS_EQUAL(0, msgq_available(msgq));
}

TEST(MessageQueue, available_ShouldReturnAvailableBytes_WhenQueueIsNotFull) {
	const size_t expected = 128-sizeof(msgq_msg_meta_t)*2-5;
	LONGS_EQUAL(128-sizeof(msgq_msg_meta_t), msgq_available(msgq));
	msgq_push(msgq, "hello", 5);
	LONGS_EQUAL(expected, msgq_available(msgq));
}

TEST(MessageQueue, available_ShouldCallSyncFunctions_WhenLockAndUnlockAreSet) {
	mock().expectOneCall("f_lock")
		.withParameter("ctx", sync_ctx);
	mock().expectOneCall("f_unlock")
		.withParameter("ctx", sync_ctx);
	msgq_set_sync(msgq, f_lock, f_unlock, sync_ctx);
	msgq_available(msgq);
}

TEST(MessageQueue, cap_ShouldCallSyncFunctions_WhenLockAndUnlockAreSet) {
	mock().expectOneCall("f_lock")
		.withParameter("ctx", sync_ctx);
	mock().expectOneCall("f_unlock")
		.withParameter("ctx", sync_ctx);
	msgq_set_sync(msgq, f_lock, f_unlock, sync_ctx);
	msgq_cap(msgq);
}

TEST(MessageQueue, len_ShouldCallSyncFunctions_WhenLockAndUnlockAreSet) {
	mock().expectOneCall("f_lock")
		.withParameter("ctx", sync_ctx);
	mock().expectOneCall("f_unlock")
		.withParameter("ctx", sync_ctx);
	msgq_set_sync(msgq, f_lock, f_unlock, sync_ctx);
	msgq_len(msgq);
}

TEST(MessageQueue, calc_size_ShouldReturnTotalBytesRequiredForNumberOfMessages) {
	struct msg { uint8_t data[4]; };
	LONGS_EQUAL(120, (sizeof(msg)+sizeof(msgq_msg_meta_t))*10);
	LONGS_EQUAL(120+8, msgq_calc_size(10, sizeof(msg)));
}

TEST(MessageQueue, calc_size_ShouldReturnRoundUpToPowerOfTwo) {
	LONGS_EQUAL(16, msgq_calc_size(1, 1));
	LONGS_EQUAL(16, msgq_calc_size(1, 8));
	LONGS_EQUAL(32, msgq_calc_size(1, 9));
}
