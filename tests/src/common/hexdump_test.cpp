#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "libmcu/hexdump.h"
#include <string.h>

TEST_GROUP(hexdump) {
	void setup(void) {
	}
	void teardown(void) {
	}
};

TEST(hexdump, ShouldReturnZero_WhenInvalidParamGiven) {
	char buf[1024];
	char data[1024];
	LONGS_EQUAL(0, hexdump_verbose(NULL, 0, NULL, 0));
	LONGS_EQUAL(0, hexdump_verbose(buf, sizeof(buf), NULL, sizeof(data)));
	LONGS_EQUAL(0, hexdump_verbose(NULL, sizeof(buf), data, sizeof(data)));
}

TEST(hexdump, ShouldReturnWrittenBytes_WhenAscii) {
	char buf[1024];
	const char *data = "16 bytes long dt";
	LONGS_EQUAL(75, hexdump_verbose(buf, sizeof(buf), data, sizeof(data)));
}

TEST(hexdump, ShouldReturnWrittenBytes_WhenBinary) {
	char buf[1024];
	const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	LONGS_EQUAL(75, hexdump_verbose(buf, sizeof(buf), data, sizeof(data)));
}

TEST(hexdump, ShouldDumpMessageIntoHexString) {
	char buf[1024];
	const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	const char *expected =
		"       0: 00 01 02 03 04 05 06 07                        \t........\n";

	LONGS_EQUAL(75, hexdump_verbose(buf, sizeof(buf), data, sizeof(data)));
	STRCMP_EQUAL(expected, buf);
}

TEST(hexdump, compute_output_size_ShouldReturnOutputSize) {
	const char *txt = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum., World!! Long stroty abcdefg";

	LONGS_EQUAL(2850, hexdump_compute_output_size(strlen(txt)));
}

TEST(hexdump, hexdump_ShouldReturnZero_WhenBufferSizeIsTooSmall) {
	uint8_t buf[8];
	const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	LONGS_EQUAL(0, hexdump_verbose(buf, sizeof(buf), data, sizeof(data)));
}

TEST(hexdump, ShouldDumpMessageOnly) {
	char buf[1024];
	const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	const char *expected = "0001020304050607";

	LONGS_EQUAL(16, hexdump(buf, sizeof(buf), data, sizeof(data)));
	STRCMP_EQUAL(expected, buf);
}
