#include "CppUTest/TestHarness.h"
#include "ringbuf.h"

TEST_GROUP(RingBuffer) {
#define SPACE_SIZE		128
	void setup(void) {
	}
	void teardown(void) {
	}

	ringbuf_t ringbuf_obj;
	uint8_t ringbuf_space[SPACE_SIZE];
	void prepare_test(void) {
		ringbuf_init(&ringbuf_obj, ringbuf_space, sizeof(ringbuf_space));
	}
};

TEST(RingBuffer, init_ShouldReturnFalse_WhenObjectIsNullOrSpaceIsNull) {
	ringbuf_t ringbuf;
	uint8_t buf[1];
	CHECK_EQUAL(false, ringbuf_init(NULL, buf, sizeof(buf)));
	CHECK_EQUAL(false, ringbuf_init(&ringbuf, NULL, sizeof(buf)));
}

TEST(RingBuffer, init_ShouldReturnTrue_WhenInitializeSuccessfully) {
	ringbuf_t ringbuf;
	uint8_t buf[1];
	CHECK_EQUAL(true, ringbuf_init(&ringbuf, buf, sizeof(buf)));
}

TEST(RingBuffer, write_ShouldReturnZero_WhenWriteSizeIsLargerThanSpaceSize) {
	prepare_test();
	uint8_t buf[129];
	CHECK_EQUAL(0, ringbuf_write(&ringbuf_obj, buf, sizeof(buf)));
}

TEST(RingBuffer, write_ShouldReturnWrittenSize_WhenWrittenSuccessfully) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	CHECK_EQUAL(sizeof(test_data),
			ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)));
	CHECK_EQUAL(sizeof(test_data), ringbuf_used(&ringbuf_obj));
}

TEST(RingBuffer, write_ShouldReturnZero_WhenFull) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	size_t written, written_total = 0;
	while ((written = ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)))) {
		written_total += written;
	}
	CHECK_EQUAL(written_total, ringbuf_used(&ringbuf_obj));
	CHECK_EQUAL(2, ringbuf_left(&ringbuf_obj));
}

TEST(RingBuffer, used_plus_left_ShouldMatchToSpaceTotalSize) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	CHECK_EQUAL(sizeof(test_data),
			ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)));
	CHECK_EQUAL(sizeof(test_data), ringbuf_used(&ringbuf_obj));
	CHECK_EQUAL(SPACE_SIZE, ringbuf_used(&ringbuf_obj) + ringbuf_left(&ringbuf_obj));
}

TEST(RingBuffer, read_ShouldReturnZero_WhenRequestSizeIsLargerThanUsedSpaceSize) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[80];
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	CHECK_EQUAL(0, ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf)));
}

TEST(RingBuffer, read_ShouldReturnDataSizeRead_WhenSuccessful) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[80] = { 0, };
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	CHECK_EQUAL(sizeof(test_data), ringbuf_read(&ringbuf_obj, 0, buf, sizeof(test_data)));
	MEMCMP_EQUAL(test_data, buf, sizeof(test_data));
}

TEST(RingBuffer, read_ShouldReturnZero_WhenRequestSizePlusOffsetIsLargerThanUsedSpaceSize) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[80] = { 0, };
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	CHECK_EQUAL(0, ringbuf_read(&ringbuf_obj, 1, buf, sizeof(test_data)));
}

TEST(RingBuffer, read_ShouldReturnDataSizeRead_WhenSuccessfulWithOffset) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	size_t data_size = sizeof(test_data);
	ringbuf_write(&ringbuf_obj, &data_size, sizeof(data_size));
	ringbuf_write(&ringbuf_obj, test_data, data_size);
	CHECK_EQUAL(data_size + sizeof(data_size), ringbuf_used(&ringbuf_obj));

	uint8_t buf[80] = { 0, };
	size_t size_read;
	CHECK_EQUAL(sizeof(size_read), ringbuf_read(&ringbuf_obj, 0, &size_read, sizeof(size_read)));
	CHECK_EQUAL(data_size, size_read);
	CHECK_EQUAL(data_size, ringbuf_read(&ringbuf_obj, sizeof(size_read), buf, size_read));
	MEMCMP_EQUAL(test_data, buf, data_size);
	CHECK_EQUAL(true, ringbuf_consume(&ringbuf_obj, data_size + sizeof(data_size)));
	CHECK_EQUAL(0, ringbuf_used(&ringbuf_obj));
}

TEST(RingBuffer, consume_ShouldReturnFalse_WhenConsumeSizeIsLargerThanUsedSpaceSize) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[80] = { 0, };
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	CHECK_EQUAL(sizeof(test_data), ringbuf_read(&ringbuf_obj, 0, buf, sizeof(test_data)));
	CHECK_EQUAL(false, ringbuf_consume(&ringbuf_obj, sizeof(test_data)+1));
	CHECK_EQUAL(sizeof(test_data), ringbuf_used(&ringbuf_obj));
}

TEST(RingBuffer, consume_ShouldReturnTrue_WhenSuccessful) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[80] = { 0, };
	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	CHECK_EQUAL(sizeof(test_data), ringbuf_read(&ringbuf_obj, 0, buf, sizeof(test_data)));
	CHECK_EQUAL(true, ringbuf_consume(&ringbuf_obj, sizeof(test_data)));
	CHECK_EQUAL(0, ringbuf_used(&ringbuf_obj));
	CHECK_EQUAL(SPACE_SIZE, ringbuf_left(&ringbuf_obj));
}

TEST(RingBuffer, read_write_ShouldWorkAsWell_WhenDataWrappedOverOffsetZero) {
	prepare_test();
	const uint8_t test_data[] = "123456789012";
	size_t written, written_total = 0;
	while ((written = ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)))) {
		written_total += written;
	}
	CHECK_EQUAL(written_total, ringbuf_used(&ringbuf_obj));
	CHECK_EQUAL(11, ringbuf_left(&ringbuf_obj));

	uint8_t buf[sizeof(test_data)];
	CHECK_EQUAL(sizeof(test_data), ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf)));
	CHECK_EQUAL(true, ringbuf_consume(&ringbuf_obj, sizeof(buf)));
	CHECK_EQUAL(written_total - sizeof(test_data), ringbuf_used(&ringbuf_obj));

	ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data));
	CHECK_EQUAL(written_total, ringbuf_used(&ringbuf_obj));

	do {
		CHECK_EQUAL(sizeof(test_data), ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf)));
		CHECK_EQUAL(true, ringbuf_consume(&ringbuf_obj, sizeof(buf)));
		MEMCMP_EQUAL(test_data, buf, sizeof(buf));
		written_total -= sizeof(buf);
	} while (written_total);

	CHECK_EQUAL(0, ringbuf_used(&ringbuf_obj));
	CHECK_EQUAL(SPACE_SIZE, ringbuf_left(&ringbuf_obj));
}

TEST(RingBuffer, write_cancel_ShouldReturnZero_WhenRequestSizeIsLargerThanSpaceUsedSize) {
	prepare_test();
	const uint8_t test_data[] = "1234567890123";
	CHECK_EQUAL(sizeof(test_data),
			ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)));
	CHECK_EQUAL(sizeof(test_data), ringbuf_used(&ringbuf_obj));
	CHECK_EQUAL(0, ringbuf_write_cancel(&ringbuf_obj, sizeof(test_data)+1));
}

TEST(RingBuffer, write_cancel_ShouldReturnCanceledSize_WhenSuccessful) {
	prepare_test();
	const uint8_t test_data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	CHECK_EQUAL(sizeof(test_data),
			ringbuf_write(&ringbuf_obj, test_data, sizeof(test_data)));
	CHECK_EQUAL(sizeof(test_data), ringbuf_used(&ringbuf_obj));
	CHECK_EQUAL(3, ringbuf_write_cancel(&ringbuf_obj, 3));
	CHECK_EQUAL(sizeof(test_data)-3, ringbuf_used(&ringbuf_obj));

	uint8_t buf[sizeof(test_data)-3];
	CHECK_EQUAL(sizeof(test_data)-3, ringbuf_read(&ringbuf_obj, 0, buf, sizeof(buf)));
	MEMCMP_EQUAL(test_data, buf, sizeof(buf));
}

TEST(RingBuffer, new_ShouldReturnNewObject) {
	ringbuf_t *obj = ringbuf_new(32);
	CHECK(obj);

	const uint8_t test_data[] = "1234567890123";
	uint8_t buf[32] = { 0, };
	ringbuf_write(obj, test_data, sizeof(test_data));
	CHECK_EQUAL(sizeof(test_data), ringbuf_read(obj, 0, buf, sizeof(test_data)));
	MEMCMP_EQUAL(test_data, buf, sizeof(test_data));

	ringbuf_delete(obj);
}