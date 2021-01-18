#include "CppUTest/TestHarness.h"

extern "C" {
#include "memory_storage.h"
#include <string.h>
}

TEST_GROUP(MemoryStorage) {
	const logging_storage_t *ops;
	uint8_t logbuf[40];

	void setup(void) {
		memset(logbuf, 0, sizeof(logbuf));
		ops = memory_storage_init(logbuf, sizeof(logbuf));
	}
	void teardown() {
		memory_storage_deinit();
	}
};

TEST(MemoryStorage, init_ShouldReturnOps) {
	CHECK(ops->write);
	CHECK(ops->read);
	CHECK(ops->consume);
	CHECK(ops->count);
}

TEST(MemoryStorage, write_ShouldReturnSizeWritten_WhenEnoughSpaceLeft) {
	const char *fixed_data = "12345678901234567890123456789012";
	size_t data_size = strlen(fixed_data);
	char buf[sizeof(logbuf)];
	for (size_t i = 0; i < data_size; i++) {
		CHECK_EQUAL(data_size, ops->write(fixed_data, data_size));
		memset(buf, 0, sizeof(buf));
		CHECK_EQUAL(data_size, ops->read(buf, data_size));
		MEMCMP_EQUAL(fixed_data, buf, data_size);
		CHECK_EQUAL(data_size, ops->consume(data_size));
	}
}

TEST(MemoryStorage, write_ShouldReturnZero_WhenNoSpaceLeft) {
	CHECK_EQUAL(0, ops->write(logbuf, sizeof(logbuf)));
}

TEST(MemoryStorage, write_ShouldReturnZero_WhenNoEnoughSpaceLeft) {
	size_t bytes_to_write = sizeof(logbuf) / 2;
	size_t left_bytes = sizeof(logbuf) - (sizeof(size_t) + bytes_to_write);
	CHECK_EQUAL(bytes_to_write, ops->write(logbuf, bytes_to_write));
	CHECK_EQUAL(0, ops->write(logbuf, left_bytes-sizeof(size_t)+1));
	CHECK_EQUAL(left_bytes - sizeof(size_t), ops->write(logbuf, left_bytes-sizeof(size_t)));
}

TEST(MemoryStorage, read_ShouldReturnZero_WhenNoDataIsThere) {
	char buf[sizeof(logbuf)];
	CHECK_EQUAL(0, ops->read(buf, sizeof(buf)));
}

TEST(MemoryStorage, read_ShouldReturnSizeRead_WhenItHasData) {
	const char *fixed_data = "12345678901234567890123456789012";
	size_t data_size = strlen(fixed_data);
	char buf[sizeof(logbuf)];
	CHECK_EQUAL(data_size, ops->write(fixed_data, data_size));
	CHECK_EQUAL(data_size, ops->read(buf, sizeof(buf)));
	MEMCMP_EQUAL(fixed_data, buf, data_size);
}

TEST(MemoryStorage, count_ShouldReturnZero_WhenNoDataIsThere) {
	CHECK_EQUAL(0, ops->count());
}

TEST(MemoryStorage, count_ShouldReturnNumberOfDataWritten) {
	const char *fixed_data = "123";
	size_t data_size = strlen(fixed_data);
	CHECK_EQUAL(data_size, ops->write(fixed_data, data_size));
	CHECK_EQUAL(1, ops->count());
	CHECK_EQUAL(data_size, ops->write(fixed_data, data_size));
	CHECK_EQUAL(2, ops->count());
	CHECK_EQUAL(data_size, ops->write(fixed_data, data_size));
	CHECK_EQUAL(3, ops->count());
}

TEST(MemoryStorage, consume_ShouldReturnZero_WhenNoData) {
	CHECK_EQUAL(0, ops->consume(1));
}

TEST(MemoryStorage, consume_ShouldReturnSizeConsumed) {
	const char *fixed_data = "123";
	size_t data_size = strlen(fixed_data);
	CHECK_EQUAL(data_size, ops->write(fixed_data, data_size));
	CHECK_EQUAL(1, ops->count());
	CHECK_EQUAL(data_size, ops->consume(data_size));
	CHECK_EQUAL(0, ops->count());
}
