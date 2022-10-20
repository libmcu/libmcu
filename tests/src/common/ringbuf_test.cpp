/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
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

TEST(RingBuffer, write_ShouldReturnZero_WhenWriteSizeIsLargerThanSpaceSize) {
	prepare_test();
	uint8_t buf[129];
	LONGS_EQUAL(0, ringbuf_write(&ringbuf_obj, buf, sizeof(buf)));
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
	LONGS_EQUAL(2, ringbuf_left(&ringbuf_obj));
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
	LONGS_EQUAL(sizeof(test_data), ringbuf_read(&ringbuf_obj, 1, buf, sizeof(test_data)));
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
	LONGS_EQUAL(11, ringbuf_left(&ringbuf_obj));

	uint8_t buf[sizeof(test_data)];
	LONGS_EQUAL(sizeof(test_data), ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf)));
	LONGS_EQUAL(written_total - sizeof(test_data), ringbuf_length(&ringbuf_obj));

	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	LONGS_EQUAL(written_total, ringbuf_length(&ringbuf_obj));

	do {
		LONGS_EQUAL(sizeof(test_data), ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf)));
		MEMCMP_EQUAL(test_data, buf, sizeof(buf));
		written_total -= sizeof(buf);
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
