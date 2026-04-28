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

TEST_GROUP(base64url) {
	void setup(void) {
	}
	void teardown(void) {
	}
};

TEST(base64, encode_ShouldReturnLengthOfEncodedData) {
	const char *fixed = "SGVsbG8sIFdvcmxkIQ==";
	const char *source = "Hello, World!";
	char buf[64];

	size_t len = lm_base64_encode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);
}

TEST(base64, decode_ShouldReturnLengthOfDecodedData) {
	const char *fixed = "Hello, World!";
	const char *source = "SGVsbG8sIFdvcmxkIQ==";
	char buf[64];

	size_t len = lm_base64_decode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);
}

TEST(base64, decode_overwrite_ShouldReturnLengthOfDecodedData) {
	const char *fixed = "Hello, World!";
	char source[] = "SGVsbG8sIFdvcmxkIQ==";

	size_t len = lm_base64_decode_overwrite(source, strlen(source), sizeof(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, source, len);
}

TEST(base64, encode_decode_a) {
	const char *fixed = "YQ==";
	const char *source = "a";
	char buf[16];

	size_t len = lm_base64_encode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);

	len = lm_base64_decode_overwrite(buf, len, sizeof(buf));
	LONGS_EQUAL(strlen(source), len);
	MEMCMP_EQUAL(source, buf, len);
}

TEST(base64, encode_decode_ab) {
	const char *fixed = "YWI=";
	const char *source = "ab";
	char buf[16];

	size_t len = lm_base64_encode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);

	len = lm_base64_decode_overwrite(buf, len, sizeof(buf));
	LONGS_EQUAL(strlen(source), len);
	MEMCMP_EQUAL(source, buf, len);
}

TEST(base64, encode_decode_abc) {
	const char *fixed = "YWJj";
	const char *source = "abc";
	char buf[16];

	size_t len = lm_base64_encode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);

	len = lm_base64_decode_overwrite(buf, len, sizeof(buf));
	LONGS_EQUAL(strlen(source), len);
	MEMCMP_EQUAL(source, buf, len);
}

TEST(base64, encode_decode_binary) {
	const char *fixed = "+EXTRA/TESTa";
	const uint8_t source[] = {
		0xf8, 0x45, 0xd3, 0x44, 0x0f, 0xd3, 0x11, 0x24, 0xda, };
	char buf[16];

	size_t len = lm_base64_encode(buf, sizeof(buf), source, sizeof(source));
	LONGS_EQUAL(strlen(fixed), len);
	MEMCMP_EQUAL(fixed, buf, len);

	len = lm_base64_decode_overwrite(buf, len, sizeof(buf));
	LONGS_EQUAL(sizeof(source), len);
	MEMCMP_EQUAL(source, buf, len);
}

TEST(base64, encode_ShouldTrimInput_WhenBufsizeIsTooSmall) {
	const char *expected = "SGVsbG8sIFdv";
	const char *source = "Hello, World!"; // "SGVsbG8sIFdvcmxkIQ=="
	char buf[12];

	size_t len = lm_base64_encode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(strlen(expected), len);
	MEMCMP_EQUAL(expected, buf, len);
}

TEST(base64, decode_ShouldTrimInput_WhenBufsizeIsTooSmall) {
	const char *expected = "Hello, Wo";
	const char *source = "SGVsbG8sIFdvcmxkIQ=="; // "Hello, World!"
	char buf[9];

	size_t len = lm_base64_decode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(strlen(expected), len);
	MEMCMP_EQUAL(expected, buf, len);
}

TEST(base64, encode_ShouldEncode_WhenBufsizeIsJustFit) {
	const char *expected = "SGVsbG8sIFdvcmxkIQ==";
	const char *source = "Hello, World!";
	char buf[20]; // length of "SGVsbG8sIFdvcmxkIQ=="

	size_t len = lm_base64_encode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(strlen(expected), len);
	MEMCMP_EQUAL(expected, buf, len);
}

TEST(base64url, encode_ShouldEncodeWithDashAndUnderscore) {
	const char *expected = "PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0idXRmLTgiPz4=";
	const char *source = "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
	char buf[64];

	size_t len = lm_base64url_encode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(strlen(expected), len);
	MEMCMP_EQUAL(expected, buf, len);
}

TEST(base64url, encode_ShouldReturnLengthOfEncodedData) {
	const char *expected = "SGVsbG8sIFdvcmxkIQ==";
	const char *source = "Hello, World!";
	char buf[64];

	size_t len = lm_base64url_encode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(strlen(expected), len);
	MEMCMP_EQUAL(expected, buf, len);
}

TEST(base64url, decode_ShouldDecodeDashAndUnderscore) {
	const char *source = "SGVsbG8sIFdvcmxkIQ";
	const char *expected = "Hello, World!";
	char buf[64];

	size_t len = lm_base64url_decode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(strlen(expected), len);
	MEMCMP_EQUAL(expected, buf, len);
}

TEST(base64url, decode_overwrite_ShouldDecodeInPlace) {
	const char *source = "SGVsbG8sIFdvcmxkIQ";
	char buf[64];
	strncpy(buf, source, strlen(source));

	size_t len = lm_base64url_decode_overwrite(buf, strlen(source), sizeof(buf));

	LONGS_EQUAL(13, len);
	MEMCMP_EQUAL("Hello, World!", buf, len);
}

TEST(base64url, encode_decode_binary) {
	const char *expected = "-EXTRA_TESTa";
	const uint8_t source[] = {
		0xf8, 0x45, 0xd3, 0x44, 0x0f, 0xd3, 0x11, 0x24, 0xda, };
	char buf[16];

	size_t len = lm_base64url_encode(buf, sizeof(buf), source, sizeof(source));
	LONGS_EQUAL(strlen(expected), len);
	MEMCMP_EQUAL(expected, buf, len);

	len = lm_base64url_decode_overwrite(buf, len, sizeof(buf));
	LONGS_EQUAL(sizeof(source), len);
	MEMCMP_EQUAL(source, buf, len);
}

TEST(base64url, decode_ShouldHandleUnpaddedSingleByte) {
	const char *source = "YQ";
	char buf[8];

	size_t len = lm_base64url_decode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(1, len);
	MEMCMP_EQUAL("a", buf, len);
}

TEST(base64url, decode_ShouldHandleUnpaddedTwoBytes) {
	const char *source = "YWI";
	char buf[8];

	size_t len = lm_base64url_decode(buf, sizeof(buf), source, strlen(source));

	LONGS_EQUAL(2, len);
	MEMCMP_EQUAL("ab", buf, len);
}

TEST(base64url, decode_overwrite_ShouldHandleUnpaddedSingleByte) {
	char buf[] = "YQ";

	size_t len = lm_base64url_decode_overwrite(buf, strlen(buf), sizeof(buf));

	LONGS_EQUAL(1, len);
	MEMCMP_EQUAL("a", buf, len);
}
