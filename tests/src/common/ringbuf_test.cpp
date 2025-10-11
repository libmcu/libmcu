/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "libmcu/ringbuf.h"

static inline size_t ringbuf_left(struct ringbuf *handle)
{
	return ringbuf_capacity(handle) - ringbuf_length(handle);
}

TEST_GROUP(RingBuffer) {
#define SPACE_SIZE		128
	void setup(void) {
	}
	void teardown(void) {
	}

	struct ringbuf ringbuf_obj;
	uint8_t ringbuf_space[SPACE_SIZE];
	void prepare_test(void) {
		ringbuf_create_static(&ringbuf_obj,
				ringbuf_space, sizeof(ringbuf_space));
	}
};

TEST(RingBuffer, init_ShouldReturnFalse_WhenObjectIsNullOrSpaceIsNull) {
	struct ringbuf ringbuf;
	uint8_t buf[2];
	CHECK_EQUAL(false, ringbuf_create_static(NULL, buf, sizeof(buf)));
	CHECK_EQUAL(false, ringbuf_create_static(&ringbuf, NULL, sizeof(buf)));
}

TEST(RingBuffer, init_ShouldReturnTrue_WhenInitializeSuccessfully) {
	struct ringbuf ringbuf;
	uint8_t buf[2];
	CHECK_EQUAL(true, ringbuf_create_static(&ringbuf, buf, sizeof(buf)));
}

TEST(RingBuffer, write_ShouldWriteAsMuchAsLeft_WhenWriteSizeIsLargerThanSpaceSize) {
	prepare_test();
	uint8_t buf[129];
	LONGS_EQUAL(128, ringbuf_write(&ringbuf_obj, buf, sizeof(buf)));
}

TEST(RingBuffer, write_ShouldReturnWrittenSize_WhenWrittenSuccessfully) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	LONGS_EQUAL(sizeof(test_data),
			ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)));
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(&ringbuf_obj));
}

TEST(RingBuffer, write_ShouldReturnZero_WhenFull) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	size_t written, written_total = 0;
	while ((written = ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)))) {
		written_total += written;
	}
	LONGS_EQUAL(written_total, ringbuf_length(&ringbuf_obj));
	LONGS_EQUAL(0, ringbuf_left(&ringbuf_obj));
}

TEST(RingBuffer, used_plus_left_ShouldMatchToSpaceTotalSize) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	LONGS_EQUAL(sizeof(test_data),
			ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)));
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(&ringbuf_obj));
	LONGS_EQUAL(SPACE_SIZE, ringbuf_length(&ringbuf_obj) + ringbuf_left(&ringbuf_obj));
}

TEST(RingBuffer, read_ShouldReturnLengthOfBytesRead_WhenLargerBufferGiven) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[80];
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	LONGS_EQUAL(sizeof(test_data), ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf)));
}

TEST(RingBuffer, read_ShouldReturnDataSizeRead_WhenSuccessful) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[80] = { 0, };
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	LONGS_EQUAL(sizeof(test_data), ringbuf_read(&ringbuf_obj, 0, buf, sizeof(test_data)));
	MEMCMP_EQUAL(test_data, buf, sizeof(test_data));
}

TEST(RingBuffer, read_ShouldReturnLengthOfBytesRead_WhenLargerBufferGivenWithOffset) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[80] = { 0, };
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	LONGS_EQUAL(sizeof(test_data)-1, ringbuf_read(&ringbuf_obj, 1, buf, sizeof(test_data)));
}

TEST(RingBuffer, peek_ShouldReturnDataSizeRead_WhenSuccessfulWithOffset) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	size_t data_size = sizeof(test_data);
	ringbuf_write(&ringbuf_obj, &data_size, sizeof(data_size));
	ringbuf_write(&ringbuf_obj, test_data, data_size);
	LONGS_EQUAL(data_size + sizeof(data_size), ringbuf_length(&ringbuf_obj));

	uint8_t buf[80] = { 0, };
	size_t size_read;
	LONGS_EQUAL(sizeof(size_read), ringbuf_peek(&ringbuf_obj, 0, &size_read, sizeof(size_read)));
	LONGS_EQUAL(data_size, size_read);
	LONGS_EQUAL(data_size, ringbuf_peek(&ringbuf_obj, sizeof(size_read), buf, size_read));
	MEMCMP_EQUAL(test_data, buf, data_size);
	CHECK_EQUAL(true, ringbuf_consume(&ringbuf_obj, data_size + sizeof(data_size)));
	LONGS_EQUAL(0, ringbuf_length(&ringbuf_obj));
}

TEST(RingBuffer, consume_ShouldReturnFalse_WhenConsumeSizeIsLargerThanUsedSpaceSize) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[80] = { 0, };
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	LONGS_EQUAL(sizeof(test_data), ringbuf_peek(&ringbuf_obj, 0, buf, sizeof(test_data)));
	CHECK_EQUAL(false, ringbuf_consume(&ringbuf_obj, sizeof(test_data)+1));
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(&ringbuf_obj));
}

TEST(RingBuffer, consume_ShouldReturnTrue_WhenSuccessful) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[80] = { 0, };
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	LONGS_EQUAL(sizeof(test_data), ringbuf_peek(&ringbuf_obj, 0, buf, sizeof(test_data)));
	CHECK_EQUAL(true, ringbuf_consume(&ringbuf_obj, sizeof(test_data)));
	LONGS_EQUAL(0, ringbuf_length(&ringbuf_obj));
	LONGS_EQUAL(SPACE_SIZE, ringbuf_left(&ringbuf_obj));
}

TEST(RingBuffer, read_write_ShouldWorkAsWell_WhenDataWrappedOverOffsetZero) {
	prepare_test();
	const uint8_t test_data[] = "123456789012";
	size_t written, written_total = 0;
	while ((written = ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)))) {
		written_total += written;
	}
	LONGS_EQUAL(written_total, ringbuf_length(&ringbuf_obj));
	LONGS_EQUAL(0, ringbuf_left(&ringbuf_obj));

	uint8_t buf[sizeof(test_data)];
	LONGS_EQUAL(sizeof(test_data), ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf)));
	LONGS_EQUAL(written_total - sizeof(test_data), ringbuf_length(&ringbuf_obj));

	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	LONGS_EQUAL(written_total, ringbuf_length(&ringbuf_obj));

	do {
		const size_t n = ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf));
		if (ringbuf_length(&ringbuf_obj) == 0) {
			MEMCMP_EQUAL("3456789012", buf, n);
		} else if (ringbuf_length(&ringbuf_obj) < sizeof(buf)) {
			MEMCMP_EQUAL("1234567890112", buf, n);
		} else {
			MEMCMP_EQUAL(test_data, buf, n);
		}
		written_total -= n;
	} while (written_total);

	LONGS_EQUAL(0, ringbuf_length(&ringbuf_obj));
	LONGS_EQUAL(SPACE_SIZE, ringbuf_left(&ringbuf_obj));
}

TEST(RingBuffer, write_cancel_ShouldReturnZero_WhenRequestSizeIsLargerThanSpaceUsedSize) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	LONGS_EQUAL(sizeof(test_data),
			ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)));
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(&ringbuf_obj));
	LONGS_EQUAL(0, ringbuf_write_cancel(&ringbuf_obj, sizeof(test_data)+1));
}

TEST(RingBuffer, write_cancel_ShouldReturnCanceledSize_WhenSuccessful) {
	prepare_test();
	const uint8_t test_data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	LONGS_EQUAL(sizeof(test_data),
			ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)));
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(&ringbuf_obj));
	LONGS_EQUAL(3, ringbuf_write_cancel(&ringbuf_obj, 3));
	LONGS_EQUAL(sizeof(test_data)-3, ringbuf_length(&ringbuf_obj));

	uint8_t buf[sizeof(test_data)-3];
	LONGS_EQUAL(sizeof(test_data)-3, ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf)));
	MEMCMP_EQUAL(test_data, buf, sizeof(buf));
}

TEST(RingBuffer, new_ShouldReturnNewObject) {
	struct ringbuf *handle = ringbuf_create(32);
	CHECK(handle);

	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[32] = { 0, };
	ringbuf_write(handle, test_data, sizeof(test_data));
	LONGS_EQUAL(sizeof(test_data), ringbuf_read(handle, 0, buf, sizeof(test_data)));
	MEMCMP_EQUAL(test_data, buf, sizeof(test_data));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, new_ShouldReturnNull_WhenOutOfMemory) {
	cpputest_malloc_set_out_of_memory();
	POINTERS_EQUAL(NULL, ringbuf_create(32));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(RingBuffer, new_ShouldReturnNull_WhenOutOfMemory2) {
	cpputest_malloc_set_out_of_memory_countdown(2);
	POINTERS_EQUAL(NULL, ringbuf_create(32));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(RingBuffer, read_ShouldConsumeOffsetAsWell_WhenOffsetIsNotZero) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[5];
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	LONGS_EQUAL(sizeof(buf), ringbuf_read(&ringbuf_obj, 5, buf, sizeof(buf)));
	LONGS_EQUAL(4, ringbuf_length(&ringbuf_obj));
}

TEST(RingBuffer, read_ShouldReturnZero_WhenOffsetIsEqualOrLargerThanUsedSize) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[5];
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	LONGS_EQUAL(0, ringbuf_read(&ringbuf_obj, sizeof(test_data), buf, sizeof(buf)));
}

TEST(RingBuffer, peek_pointer_ShouldReturnNull_WhenOffsetIsLargerThanUsedSize) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	POINTERS_EQUAL(NULL, ringbuf_peek_pointer(&ringbuf_obj, sizeof(test_data)+1, NULL));
}

TEST(RingBuffer, peek_pointer_ShouldReturnNull_WhenOffsetIsSameAsUsedSize) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	POINTERS_EQUAL(NULL, ringbuf_peek_pointer(&ringbuf_obj, sizeof(test_data), NULL));
}

TEST(RingBuffer, peek_pointer_ShouldReturnPointer_WhenSuccessful) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	size_t contiguous;
	const uint8_t *ptr = (const uint8_t *)ringbuf_peek_pointer(&ringbuf_obj, 0, &contiguous);
	POINTERS_EQUAL(ringbuf_space, ptr);
	LONGS_EQUAL(sizeof(test_data), contiguous);
}

TEST(RingBuffer, peek_pointer_ShouldReturnPointer_WhenOffsetIsNotZero) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	size_t contiguous;
	const uint8_t *ptr = (const uint8_t *)ringbuf_peek_pointer(&ringbuf_obj, 5, &contiguous);
	POINTERS_EQUAL(ringbuf_space+5, ptr);
	LONGS_EQUAL(sizeof(test_data)-5, contiguous);
}

TEST(RingBuffer, peek_pointer_ShouldReturnPointer_WhenNullSizePointerGiven) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	const uint8_t *ptr = (const uint8_t *)ringbuf_peek_pointer(&ringbuf_obj, 0, NULL);
	POINTERS_EQUAL(ringbuf_space, ptr);
}

TEST(RingBuffer, peek_pointer_ShouldSetOnlyContiguousSize_WhenWraparound) {
	prepare_test();
	const uint8_t test_data[] = "123456789012";
	while (ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)));
	while (1) {
		uint8_t buf[sizeof(test_data)];
		ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf));
		if (ringbuf_length(&ringbuf_obj) < sizeof(buf)) {
			break;
		}
	};

	size_t contiguous;
	const uint8_t *ptr = (const uint8_t *)ringbuf_peek_pointer(&ringbuf_obj, 0, &contiguous);
	POINTERS_EQUAL(&ringbuf_space[SPACE_SIZE-11], ptr);
	LONGS_EQUAL(11, contiguous);
}

TEST(RingBuffer, read_ShouldReturnZero_WhenRingBufferIsEmpty) {
	prepare_test();
	uint8_t buf[5];
	LONGS_EQUAL(0, ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf)));
}

TEST(RingBuffer, peek_ShouldReturnZero_WhenRingBufferIsEmpty) {
	prepare_test();
	uint8_t buf[5];
	LONGS_EQUAL(0, ringbuf_peek(&ringbuf_obj, 0, buf, sizeof(buf)));
}

TEST(RingBuffer, create_ShouldRoundUpToPowerOfTwo_WhenSizeIsNotPowerOfTwo) {
	/* Request 100 bytes, should get 128 bytes (rounded up) */
	struct ringbuf *handle = ringbuf_create(100);
	CHECK(handle);
	LONGS_EQUAL(128, ringbuf_capacity(handle));
	ringbuf_destroy(handle);
}

TEST(RingBuffer, create_ShouldKeepSizeUnchanged_WhenSizeIsPowerOfTwo) {
	/* Request 128 bytes (already power of 2), should get 128 bytes */
	struct ringbuf *handle = ringbuf_create(128);
	CHECK(handle);
	LONGS_EQUAL(128, ringbuf_capacity(handle));
	ringbuf_destroy(handle);
}

TEST(RingBuffer, create_ShouldRoundUpCorrectly_WhenSizeIs1) {
	/* Request 1 byte, should get 1 byte (already power of 2) */
	struct ringbuf *handle = ringbuf_create(1);
	CHECK(handle);
	LONGS_EQUAL(1, ringbuf_capacity(handle));
	ringbuf_destroy(handle);
}

TEST(RingBuffer, create_ShouldRoundUpCorrectly_WhenSizeIs150) {
	/* Request 150 bytes, should get 256 bytes (rounded up) */
	struct ringbuf *handle = ringbuf_create(150);
	CHECK(handle);
	LONGS_EQUAL(256, ringbuf_capacity(handle));
	ringbuf_destroy(handle);
}

TEST(RingBuffer, create_static_ShouldReturnFalse_WhenSizeIsNotPowerOfTwo) {
	/* Static buffer with non-power-of-2 size should fail */
	struct ringbuf ringbuf;
	uint8_t buf[100];
	CHECK_EQUAL(false, ringbuf_create_static(&ringbuf, buf, sizeof(buf)));
}

TEST(RingBuffer, create_static_ShouldReturnTrue_WhenSizeIsPowerOfTwo) {
	/* Static buffer with power-of-2 size should succeed */
	struct ringbuf ringbuf;
	uint8_t buf[128];
	CHECK_EQUAL(true, ringbuf_create_static(&ringbuf, buf, sizeof(buf)));
	LONGS_EQUAL(128, ringbuf_capacity(&ringbuf));
}

TEST(RingBuffer, create_static_ShouldReturnTrue_WhenSizeIs1) {
	/* Static buffer with size 1 (power of 2) should succeed */
	struct ringbuf ringbuf;
	uint8_t buf[1];
	CHECK_EQUAL(true, ringbuf_create_static(&ringbuf, buf, sizeof(buf)));
	LONGS_EQUAL(1, ringbuf_capacity(&ringbuf));
}

TEST(RingBuffer, create_static_ShouldReturnFalse_WhenSizeIs3) {
	/* Static buffer with size 3 (not power of 2) should fail */
	struct ringbuf ringbuf;
	uint8_t buf[3];
	CHECK_EQUAL(false, ringbuf_create_static(&ringbuf, buf, sizeof(buf)));
}

TEST(RingBuffer, resize_ShouldReturnFalse_WhenHandleIsNull) {
	CHECK_EQUAL(false, ringbuf_resize(NULL, 256));
}

TEST(RingBuffer, resize_ShouldReturnFalse_WhenNewSizeIsZero) {
	struct ringbuf *handle = ringbuf_create(128);
	CHECK_EQUAL(false, ringbuf_resize(handle, 0));
	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldReturnTrue_WhenSizeIsUnchanged) {
	/* Resize to same size (128) should succeed without doing anything */
	struct ringbuf *handle = ringbuf_create(128);
	CHECK_EQUAL(true, ringbuf_resize(handle, 128));
	LONGS_EQUAL(128, ringbuf_capacity(handle));
	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldExpandBuffer_WhenNewSizeIsLarger) {
	/* Expand from 128 to 256 bytes */
	struct ringbuf *handle = ringbuf_create(128);
	const uint8_t test_data[] = "1234567890123";

	ringbuf_write(handle, test_data, sizeof(test_data));
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(handle));

	/* Resize to 256 */
	CHECK_EQUAL(true, ringbuf_resize(handle, 256));
	LONGS_EQUAL(256, ringbuf_capacity(handle));

	/* Data should be preserved */
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(handle));
	uint8_t buf[sizeof(test_data)];
	LONGS_EQUAL(sizeof(test_data), ringbuf_read(handle, 0, buf, sizeof(buf)));
	MEMCMP_EQUAL(test_data, buf, sizeof(test_data));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldShrinkBuffer_WhenNewSizeIsSmaller) {
	/* Shrink from 256 to 128 bytes (with data that fits) */
	struct ringbuf *handle = ringbuf_create(256);
	const uint8_t test_data[] = "1234567890123";

	ringbuf_write(handle, test_data, sizeof(test_data));
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(handle));

	/* Resize to 128 */
	CHECK_EQUAL(true, ringbuf_resize(handle, 128));
	LONGS_EQUAL(128, ringbuf_capacity(handle));

	/* Data should be preserved */
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(handle));
	uint8_t buf[sizeof(test_data)];
	LONGS_EQUAL(sizeof(test_data), ringbuf_read(handle, 0, buf, sizeof(buf)));
	MEMCMP_EQUAL(test_data, buf, sizeof(test_data));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldReturnFalse_WhenShrinkingBelowDataSize) {
	/* Try to shrink below current data size */
	struct ringbuf *handle = ringbuf_create(256);
	uint8_t test_data[200];
	memset(test_data, 0xAA, sizeof(test_data));

	ringbuf_write(handle, test_data, sizeof(test_data));
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(handle));

	/* Try to resize to 128 (smaller than 200 bytes of data) */
	CHECK_EQUAL(false, ringbuf_resize(handle, 128));

	/* Buffer should remain unchanged */
	LONGS_EQUAL(256, ringbuf_capacity(handle));
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(handle));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldWorkWithEmptyBuffer) {
	/* Resize empty buffer */
	struct ringbuf *handle = ringbuf_create(128);
	LONGS_EQUAL(0, ringbuf_length(handle));

	CHECK_EQUAL(true, ringbuf_resize(handle, 256));
	LONGS_EQUAL(256, ringbuf_capacity(handle));
	LONGS_EQUAL(0, ringbuf_length(handle));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldPreserveDataWithFullBuffer) {
	/* Resize when buffer is full */
	struct ringbuf *handle = ringbuf_create(128);
	uint8_t test_data[128];
	for (size_t i = 0; i < sizeof(test_data); i++) {
		test_data[i] = (uint8_t)i;
	}

	ringbuf_write(handle, test_data, sizeof(test_data));
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(handle));

	/* Resize to 256 */
	CHECK_EQUAL(true, ringbuf_resize(handle, 256));
	LONGS_EQUAL(256, ringbuf_capacity(handle));

	/* All data should be preserved */
	LONGS_EQUAL(sizeof(test_data), ringbuf_length(handle));
	uint8_t buf[sizeof(test_data)];
	LONGS_EQUAL(sizeof(test_data), ringbuf_read(handle, 0, buf, sizeof(buf)));
	MEMCMP_EQUAL(test_data, buf, sizeof(test_data));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldPreserveDataWithWraparound) {
	/* Resize when data is wrapped around */
	struct ringbuf *handle = ringbuf_create(128);
	const uint8_t test_data[] = "123456789012";

	/* Fill buffer */
	while (ringbuf_write(handle, test_data, sizeof(test_data)));

	/* Read some data to create wraparound */
	uint8_t buf[sizeof(test_data)];
	ringbuf_read(handle, 0, buf, sizeof(buf));

	/* Write again to create wraparound */
	size_t written = ringbuf_write(handle, test_data, sizeof(test_data));
	CHECK(written > 0);

	const size_t data_len_before = ringbuf_length(handle);

	/* Resize to 256 */
	CHECK_EQUAL(true, ringbuf_resize(handle, 256));
	LONGS_EQUAL(256, ringbuf_capacity(handle));

	/* Data length should be preserved */
	LONGS_EQUAL(data_len_before, ringbuf_length(handle));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldRoundUpToNextPowerOfTwo) {
	/* Request 200 bytes, should get 256 */
	struct ringbuf *handle = ringbuf_create(128);
	CHECK_EQUAL(true, ringbuf_resize(handle, 200));
	LONGS_EQUAL(256, ringbuf_capacity(handle));
	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldReturnFalse_WhenOutOfMemory) {
	struct ringbuf *handle = ringbuf_create(128);
	cpputest_malloc_set_out_of_memory();
	CHECK_EQUAL(false, ringbuf_resize(handle, 256));
	cpputest_malloc_set_not_out_of_memory();

	/* Buffer should remain unchanged */
	LONGS_EQUAL(128, ringbuf_capacity(handle));
	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldPreserveRemainingData_WhenPartiallyConsumed) {
	/* Test: Write data -> Read partial -> Resize -> Check remaining data */
	struct ringbuf *handle = ringbuf_create(128);
	const uint8_t test_data[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	const size_t data_size = sizeof(test_data);

	/* Write 63 bytes */
	ringbuf_write(handle, test_data, data_size);
	LONGS_EQUAL(data_size, ringbuf_length(handle));

	/* Read and consume only 30 bytes */
	uint8_t buf_partial[30];
	LONGS_EQUAL(sizeof(buf_partial), ringbuf_read(handle, 0, buf_partial, sizeof(buf_partial)));
	MEMCMP_EQUAL(test_data, buf_partial, sizeof(buf_partial));

	/* Now 33 bytes should remain (63 - 30) */
	const size_t remaining = data_size - sizeof(buf_partial);
	LONGS_EQUAL(remaining, ringbuf_length(handle));

	/* Resize to 256 */
	CHECK_EQUAL(true, ringbuf_resize(handle, 256));
	LONGS_EQUAL(256, ringbuf_capacity(handle));

	/* Remaining data should still be 33 bytes */
	LONGS_EQUAL(remaining, ringbuf_length(handle));

	/* Read remaining data and verify it matches the unconsumed portion */
	uint8_t buf_remaining[remaining];
	LONGS_EQUAL(remaining, ringbuf_read(handle, 0, buf_remaining, sizeof(buf_remaining)));
	MEMCMP_EQUAL(test_data + sizeof(buf_partial), buf_remaining, remaining);

	/* Buffer should now be empty */
	LONGS_EQUAL(0, ringbuf_length(handle));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldHandleComplexScenario_WriteReadWriteResize) {
	/* Complex scenario: Write -> Read partial -> Write more -> Resize */
	struct ringbuf *handle = ringbuf_create(128);
	const uint8_t data1[] = "First batch of data with 50 bytes total here!";
	const uint8_t data2[] = "Second batch";

	/* Write first batch (47 bytes) */
	ringbuf_write(handle, data1, sizeof(data1));
	LONGS_EQUAL(sizeof(data1), ringbuf_length(handle));

	/* Consume 20 bytes */
	uint8_t buf[50];
	LONGS_EQUAL(20, ringbuf_read(handle, 0, buf, 20));
	MEMCMP_EQUAL(data1, buf, 20);
	LONGS_EQUAL(sizeof(data1) - 20, ringbuf_length(handle));

	/* Write second batch (13 bytes) */
	ringbuf_write(handle, data2, sizeof(data2));
	const size_t total_remaining = sizeof(data1) - 20 + sizeof(data2);
	LONGS_EQUAL(total_remaining, ringbuf_length(handle));

	/* Resize to 256 */
	CHECK_EQUAL(true, ringbuf_resize(handle, 256));
	LONGS_EQUAL(256, ringbuf_capacity(handle));

	/* Data length should be preserved */
	LONGS_EQUAL(total_remaining, ringbuf_length(handle));

	/* Verify first remaining part from data1 */
	uint8_t verify_buf[total_remaining];
	LONGS_EQUAL(total_remaining, ringbuf_read(handle, 0, verify_buf, sizeof(verify_buf)));

	/* Should have remaining part of data1 followed by data2 */
	MEMCMP_EQUAL(data1 + 20, verify_buf, sizeof(data1) - 20);
	MEMCMP_EQUAL(data2, verify_buf + sizeof(data1) - 20, sizeof(data2));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, resize_ShouldWorkCorrectly_WhenIndexWrapsAround) {
	/* Test with index/outdex values that have wrapped around capacity */
	struct ringbuf *handle = ringbuf_create(128);
	const uint8_t pattern[] = "0123456789ABCDEF"; /* 17 bytes */

	/* Fill buffer multiple times to make index/outdex large */
	for (int i = 0; i < 20; i++) {
		/* Write 17 bytes */
		ringbuf_write(handle, pattern, sizeof(pattern));
		/* Read 17 bytes */
		uint8_t buf[sizeof(pattern)];
		ringbuf_read(handle, 0, buf, sizeof(buf));
	}

	/* At this point, index and outdex are both around 340 (20 * 17)
	 * but buffer is empty */
	LONGS_EQUAL(0, ringbuf_length(handle));

	/* Now write some data */
	const uint8_t final_data[] = "FINAL TEST DATA";
	ringbuf_write(handle, final_data, sizeof(final_data));
	LONGS_EQUAL(sizeof(final_data), ringbuf_length(handle));

	/* Resize */
	CHECK_EQUAL(true, ringbuf_resize(handle, 256));
	LONGS_EQUAL(256, ringbuf_capacity(handle));

	/* Data should be preserved */
	LONGS_EQUAL(sizeof(final_data), ringbuf_length(handle));
	uint8_t buf[sizeof(final_data)];
	LONGS_EQUAL(sizeof(final_data), ringbuf_read(handle, 0, buf, sizeof(buf)));
	MEMCMP_EQUAL(final_data, buf, sizeof(final_data));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, available_ShouldReturnCapacity_WhenBufferIsEmpty) {
	struct ringbuf *handle = ringbuf_create(128);
	LONGS_EQUAL(128, ringbuf_capacity(handle));
	LONGS_EQUAL(0, ringbuf_length(handle));
	LONGS_EQUAL(128, ringbuf_available(handle));
	ringbuf_destroy(handle);
}

TEST(RingBuffer, available_ShouldReturnZero_WhenBufferIsFull) {
	struct ringbuf *handle = ringbuf_create(128);
	uint8_t data[128];
	memset(data, 0xAA, sizeof(data));

	ringbuf_write(handle, data, sizeof(data));
	LONGS_EQUAL(128, ringbuf_length(handle));
	LONGS_EQUAL(0, ringbuf_available(handle));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, available_ShouldEqualCapacityMinusLength) {
	struct ringbuf *handle = ringbuf_create(128);
	const uint8_t test_data[] = "Test data with 50 bytes in total for testing!";

	ringbuf_write(handle, test_data, sizeof(test_data));
	const size_t length = ringbuf_length(handle);
	const size_t capacity = ringbuf_capacity(handle);
	const size_t available = ringbuf_available(handle);

	LONGS_EQUAL(capacity - length, available);
	LONGS_EQUAL(128 - sizeof(test_data), available);

	ringbuf_destroy(handle);
}

TEST(RingBuffer, available_ShouldUpdateAfterWrite) {
	struct ringbuf *handle = ringbuf_create(128);

	LONGS_EQUAL(128, ringbuf_available(handle));

	const uint8_t data1[] = "First write";
	ringbuf_write(handle, data1, sizeof(data1));
	LONGS_EQUAL(128 - sizeof(data1), ringbuf_available(handle));

	const uint8_t data2[] = "Second write";
	ringbuf_write(handle, data2, sizeof(data2));
	LONGS_EQUAL(128 - sizeof(data1) - sizeof(data2), ringbuf_available(handle));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, available_ShouldUpdateAfterRead) {
	struct ringbuf *handle = ringbuf_create(128);
	const uint8_t test_data[] = "Test data for reading and checking availability";

	ringbuf_write(handle, test_data, sizeof(test_data));
	LONGS_EQUAL(128 - sizeof(test_data), ringbuf_available(handle));

	/* Read half of the data */
	uint8_t buf[25];
	ringbuf_read(handle, 0, buf, sizeof(buf));
	LONGS_EQUAL(128 - sizeof(test_data) + sizeof(buf), ringbuf_available(handle));

	ringbuf_destroy(handle);
}

TEST(RingBuffer, available_ShouldWorkWithStaticBuffer) {
	struct ringbuf ringbuf;
	uint8_t buffer[64];

	ringbuf_create_static(&ringbuf, buffer, sizeof(buffer));
	LONGS_EQUAL(64, ringbuf_available(&ringbuf));

	const uint8_t test_data[] = "Static buffer test";
	ringbuf_write(&ringbuf, test_data, sizeof(test_data));
	LONGS_EQUAL(64 - sizeof(test_data), ringbuf_available(&ringbuf));
}

TEST(RingBuffer, available_ShouldUpdateAfterResize) {
	struct ringbuf *handle = ringbuf_create(128);
	const uint8_t test_data[] = "Data before resize";

	ringbuf_write(handle, test_data, sizeof(test_data));
	LONGS_EQUAL(128 - sizeof(test_data), ringbuf_available(handle));

	/* Resize to 256 */
	ringbuf_resize(handle, 256);
	LONGS_EQUAL(256 - sizeof(test_data), ringbuf_available(handle));

	ringbuf_destroy(handle);
}
