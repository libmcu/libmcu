/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

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

TEST(base64, encode_decode_a) {
	const char *fixed = "YQ==";
	const char *source = "a";
	char buf[16];

	size_t len = base64_encode(buf, source, strlen(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);

	len = base64_decode_overwrite(buf, len);
	LONGS_EQUAL(strlen(source), len);
	MEMCMP_EQUAL(source, buf, len);
}

TEST(base64, encode_decode_ab) {
	const char *fixed = "YWI=";
	const char *source = "ab";
	char buf[16];

	size_t len = base64_encode(buf, source, strlen(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);

	len = base64_decode_overwrite(buf, len);
	LONGS_EQUAL(strlen(source), len);
	MEMCMP_EQUAL(source, buf, len);
}

TEST(base64, encode_decode_abc) {
	const char *fixed = "YWJj";
	const char *source = "abc";
	char buf[16];

	size_t len = base64_encode(buf, source, strlen(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);

	len = base64_decode_overwrite(buf, len);
	LONGS_EQUAL(strlen(source), len);
	MEMCMP_EQUAL(source, buf, len);
}

TEST(base64, encode_decode_binary) {
	const char *fixed = "+EXTRA/TESTa";
	const uint8_t source[] = {
		0xf8, 0x45, 0xd3, 0x44, 0x0f, 0xd3, 0x11, 0x24, 0xda, };
	char buf[16];

	size_t len = base64_encode(buf, source, sizeof(source));
	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);

	len = base64_decode_overwrite(buf, len);
	LONGS_EQUAL(sizeof(source), len);
	MEMCMP_EQUAL(source, buf, len);
}
