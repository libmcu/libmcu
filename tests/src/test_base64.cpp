#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

#include <string.h>
#include "libmcu/base64.h"

TEST_GROUP(base64) {
	void setup(void) {
	}
	void teardown(void) {
	}
};

TEST(base64, encode_ShouldReturnLengthOfEncodedData) {
	const char *fixed = "SGVsbG8sIFdvcmxkIQ==";
	const char *source = "Hello, World!";
	char buf[64];

	size_t len = base64_encode(buf, source, strlen(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);
}

TEST(base64, decode_ShouldReturnLengthOfDecodedData) {
	const char *fixed = "Hello, World!";
	const char *source = "SGVsbG8sIFdvcmxkIQ==";
	char buf[64];

	size_t len = base64_decode(buf, source, strlen(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);
}

TEST(base64, decode_overwrite_ShouldReturnLengthOfDecodedData) {
	const char *fixed = "Hello, World!";
	char source[] = "SGVsbG8sIFdvcmxkIQ==";

	size_t len = base64_decode_overwrite(source, strlen(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, source, len);
}
