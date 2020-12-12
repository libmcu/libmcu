#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include <string.h>
#include "libmcu/logging.h"
#include "stubs/memory_storage.h"

static time_t fake_time;
time_t time(time_t *ptr) {
	(void)ptr;
	return mock().actualCall(__func__)
		.returnLongIntValueOrDefault(fake_time++);
}

TEST_GROUP(logging) {
#define LOGBUF_SIZE	256
	uint8_t logbuf[LOGBUF_SIZE];

	void setup(void) {
		mock().ignoreOtherCalls();
		memset(logbuf, 0, sizeof(logbuf));
		logging_init(memory_storage_init(logbuf, sizeof(logbuf)));
	}
	void teardown() {
		memory_storage_deinit();
		mock().checkExpectations();
		mock().clear();
	}

	void *fake_get_lr(void) {
		static uintptr_t fake_lr = 0xDEADBEEF;
		return (void *)fake_lr++;
	}
	void *fake_get_pc(void) {
		static uintptr_t fake_pc = 0x11223344;
		return (void *)fake_pc++;
	}

	size_t write_empty_log(logging_t type) {
		return logging_save(type, fake_get_pc(), fake_get_lr(), "");
	}
	size_t write_log_with_string(logging_t type, const char *str) {
		return logging_save(type, fake_get_pc(), fake_get_lr(), str);
	}

	size_t save_full(void) {
		size_t written, written_total = 0;
		while ((written = write_empty_log(LOGGING_TYPE_DEBUG))) {
			written_total += written;
		}
		return written_total;
	}
	size_t read_empty(void) {
		size_t bytes_read, bytes_read_total = 0;
		uint8_t buf[LOGBUF_SIZE];
		while ((bytes_read = logging_peek(buf, sizeof(buf)))) {
			bytes_read_total += bytes_read;
			CHECK_EQUAL(bytes_read, logging_consume(bytes_read));
		}
		return bytes_read_total;
	}
};

TEST(logging, get_level_ShouldReturnDebug_WhenInitialized) {
	LONGS_EQUAL(LOGGING_TYPE_DEBUG, logging_get_level());
}

TEST(logging, get_level_ShouldReturnCurrentLogLevel) {
	logging_set_level(LOGGING_TYPE_ERROR);
	LONGS_EQUAL(LOGGING_TYPE_ERROR, logging_get_level());
}

TEST(logging, set_level_ShouldIgnore_WhenInvalidLevelGiven) {
	logging_set_level(LOGGING_TYPE_MAX);
	LONGS_EQUAL(LOGGING_TYPE_DEBUG, logging_get_level());

	logging_set_level((logging_t)-1);
	LONGS_EQUAL(LOGGING_TYPE_DEBUG, logging_get_level());
}

TEST(logging, count_ShouldReturnZero_WhenNologsSaved) {
	LONGS_EQUAL(0, logging_count());
}

TEST(logging, count_ShouldReturnNumberOfLogs) {
	write_empty_log(LOGGING_TYPE_INFO);
	LONGS_EQUAL(1, logging_count());
	write_empty_log(LOGGING_TYPE_INFO);
	LONGS_EQUAL(2, logging_count());
	write_empty_log(LOGGING_TYPE_INFO);
	LONGS_EQUAL(3, logging_count());

	uint8_t buf[LOGBUF_SIZE];
	logging_read(buf, sizeof(buf));
	LONGS_EQUAL(2, logging_count());
	logging_read(buf, sizeof(buf));
	LONGS_EQUAL(1, logging_count());
	logging_read(buf, sizeof(buf));
	LONGS_EQUAL(0, logging_count());
}

TEST(logging, write_ShouldReturnZero_WhenLogLevelIsLowerThanMinSaveLevel) {
	logging_set_level(LOGGING_TYPE_ERROR);
	LONGS_EQUAL(0, write_empty_log(LOGGING_TYPE_DEBUG));
}

TEST(logging, write_ShouldReturnZero_WhenLogLevelIsInvalid) {
	LONGS_EQUAL(0, write_empty_log(LOGGING_TYPE_MAX));
}

TEST(logging, write_ShouldReturnWrittenSize) {
	size_t written = write_empty_log(LOGGING_TYPE_DEBUG);
	CHECK(written > 0);
}

TEST(logging, write_ShouldReturnZero_WhenStorageFull) {
	size_t written_total = 0;
	size_t written;

	while ((written = write_empty_log(LOGGING_TYPE_DEBUG))) {
		written_total += written;
	}

	LONGS_EQUAL(0, written);
	CHECK(written_total <= sizeof(logbuf));
}

TEST(logging, write_ShouldSaveCurrentTime) {
	uint32_t expected_time = 1234567;
	mock().expectOneCall("time").andReturnValue(expected_time);
	size_t written = write_empty_log(LOGGING_TYPE_DEBUG);
	CHECK(written > 0);

	uint8_t buf[32];
	LONGS_EQUAL(written, logging_peek(buf, sizeof(buf)));
	uint32_t actual_time = *(uint32_t *)buf;
	LONGS_EQUAL(expected_time, actual_time);
	LONGS_EQUAL(written, logging_consume(written));

	// try one more log just in case
	mock().expectOneCall("time").andReturnValue(expected_time+1);
	CHECK((written = write_empty_log(LOGGING_TYPE_DEBUG)) > 0);
	LONGS_EQUAL(written, logging_peek(buf, sizeof(buf)));
	actual_time = *(uint32_t *)buf;
	LONGS_EQUAL(expected_time+1, actual_time);
	LONGS_EQUAL(written, logging_consume(written));
}

TEST(logging, read_ShouldReturnZero_WhenBufIsNull) {
	LONGS_EQUAL(0, logging_read(NULL, 100));
}

TEST(logging, read_ShouldReturnZero_WhenBufSizeNotEnough) {
	uint8_t buf[1];
	write_empty_log(LOGGING_TYPE_DEBUG);
	LONGS_EQUAL(0, logging_read(buf, sizeof(buf)));
}

TEST(logging, read_ShouldReturnZero_WhenNoLogs) {
	uint8_t buf[LOGBUF_SIZE];
	LONGS_EQUAL(0, logging_read(buf, sizeof(buf)));
}

TEST(logging, read_ShouldReturnLogSize) {
	uint8_t buf[LOGBUF_SIZE];
	size_t bytes_written = write_empty_log(LOGGING_TYPE_DEBUG);
	size_t bytes_read = logging_read(buf, sizeof(buf));
	LONGS_EQUAL(bytes_written, bytes_read);
}

TEST(logging, write_ShouldWriteTime) {
	uint8_t buf[LOGBUF_SIZE];
	uint32_t expected_time = 12345;
	mock().expectOneCall("time").andReturnValue(expected_time);
	write_empty_log(LOGGING_TYPE_DEBUG);
	logging_read(buf, sizeof(buf));
	uint32_t actual_time = *(uint32_t *)buf;
	LONGS_EQUAL(expected_time, actual_time);
}

TEST(logging, write_read_RepeatOver) {
	uint8_t buf[LOGBUF_SIZE];
	size_t bytes_written, bytes_read;
	uint32_t actual_time;
	uint32_t expected_time = 1;

	for (uint32_t i = 0; i < sizeof(buf)*2; i++) {
		mock().expectOneCall("time").andReturnValue(expected_time+i);
		CHECK((bytes_written = write_empty_log(LOGGING_TYPE_DEBUG)) > 0);
		bytes_read = logging_peek(buf, sizeof(buf));
		LONGS_EQUAL(bytes_written, bytes_read);
		actual_time = *(uint32_t *)buf;
		LONGS_EQUAL(expected_time+i, actual_time);
		LONGS_EQUAL(bytes_read, logging_consume(bytes_read));
		bytes_read = logging_read(buf, sizeof(buf));
		LONGS_EQUAL(0, bytes_read);
	}
}

TEST(logging, write_read_ShouldKeepSameNumberOfLogs_WhenRepeatWriteFullAndReadEmpty) {
	size_t written_total, read_total;

	size_t written_total_org = save_full();
	read_total = read_empty();

	for (int i = 0; i < 100; i++) {
		written_total = save_full();
		read_total = read_empty();
		LONGS_EQUAL(written_total, read_total);
		LONGS_EQUAL(written_total, written_total_org);
	}
}

TEST(logging, write_ShouldSaveMessage_WhenPutMessage) {
	const char *fixed_message = "Hello";
	size_t written = write_log_with_string(LOGGING_TYPE_INFO, fixed_message);
	uint8_t buf[LOGBUF_SIZE];
	size_t bytes_read = logging_read(buf, sizeof(buf));
	LONGS_EQUAL(written, bytes_read);
	MEMCMP_EQUAL(&buf[bytes_read - strlen(fixed_message)],
			fixed_message, strlen(fixed_message));
}

TEST(logging, write_ShouldSaveVariable_WhenPutMessageWithVariable) {
	const char *fixed_message = "World";
	size_t written = logging_save(LOGGING_TYPE_INFO,
			fake_get_pc(), fake_get_lr(), "%s", fixed_message);
	uint8_t buf[LOGBUF_SIZE];
	size_t bytes_read = logging_read(buf, sizeof(buf));
	LONGS_EQUAL(written, bytes_read);
	MEMCMP_EQUAL(&buf[bytes_read - strlen(fixed_message)],
			fixed_message, strlen(fixed_message));
}

TEST(logging, write_ShouldReturnZero_WhenTypeIsUnknown) {
	const char *fixed_message = "Fixed Message";
	size_t written = logging_save(LOGGING_TYPE_MAX,
			fake_get_pc(), fake_get_lr(), "%s", fixed_message);
	LONGS_EQUAL(0, written);
}

TEST(logging, ShouldTakeCareOfNullPointer_WhenNullMessageDelivered) {
	size_t written = logging_save(LOGGING_TYPE_INFO,
			fake_get_pc(), fake_get_lr(), NULL);
	uint8_t buf[LOGBUF_SIZE];
	size_t bytes_read = logging_read(buf, sizeof(buf));
	LONGS_EQUAL(written, bytes_read);
}
