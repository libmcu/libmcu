/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/strext.h"

static void on_strchunk(const char *chunk, size_t chunk_len, void *ctx) {
	mock().actualCall("on_strchunk")
		.withStringParameter("chunk", chunk)
		.withParameter("chunk_len", chunk_len);
}

TEST_GROUP(STREXT) {
	void setup(void) {
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(STREXT, strtrim_ShouldRemoveLeadingSpaces) {
	char s1[] = "   remove leading spaces";
	strtrim(s1, ' ');
	STRCMP_EQUAL("remove leading spaces", s1);
}

TEST(STREXT, strtrim_ShouldRemoveTrailingSpaces) {
	char s1[] = "remove trailing spaces      ";
	strtrim(s1, ' ');
	STRCMP_EQUAL("remove trailing spaces", s1);
}

TEST(STREXT, strtrim_ShouldRemoveLeadingAndTrailingSpaces) {
	char s1[] = "     remove leading and trailing spaces      ";
	strtrim(s1, ' ');
	STRCMP_EQUAL("remove leading and trailing spaces", s1);
}

TEST(STREXT, strupper_ShouldConvertLowercaseToUppercase) {
	char s1[] = "hello, world!";
	strupper(s1);
	STRCMP_EQUAL("HELLO, WORLD!", s1);
}

TEST(STREXT, strlower_ShouldConvertUppercaseToLowercase) {
	char s1[] = "HELLO, WORLD!";
	strlower(s1);
	STRCMP_EQUAL("hello, world!", s1);
}

TEST(STREXT, strchunk_ShouldCallCallbackForEachChunk) {
	const char *str = "a,b2,c34";
	const char delimiter = ',';

	mock().expectOneCall("on_strchunk")
		.withStringParameter("chunk", str)
		.withParameter("chunk_len", 1);
	mock().expectOneCall("on_strchunk")
		.withStringParameter("chunk", &str[2])
		.withParameter("chunk_len", 2);
	mock().expectOneCall("on_strchunk")
		.withStringParameter("chunk", &str[5])
		.withParameter("chunk_len", 3);
	LONGS_EQUAL(3, strchunk(str, delimiter, on_strchunk, NULL));
}

TEST(STREXT, strchunk_ShouldNotProcessTrailingDelimiter) {
	const char *str = "a,b2,c34,";
	const char delimiter = ',';

	mock().expectOneCall("on_strchunk")
		.withStringParameter("chunk", str)
		.withParameter("chunk_len", 1);
	mock().expectOneCall("on_strchunk")
		.withStringParameter("chunk", &str[2])
		.withParameter("chunk_len", 2);
	mock().expectOneCall("on_strchunk")
		.withStringParameter("chunk", &str[5])
		.withParameter("chunk_len", 3);
	LONGS_EQUAL(3, strchunk(str, delimiter, on_strchunk, NULL));
}

TEST(STREXT, strchunk_ShouldNotProcessLeadingDelimiter) {
	const char *str = ",a,b2,c34";
	const char delimiter = ',';

	mock().expectOneCall("on_strchunk")
		.withStringParameter("chunk", &str[1])
		.withParameter("chunk_len", 1);
	mock().expectOneCall("on_strchunk")
		.withStringParameter("chunk", &str[3])
		.withParameter("chunk_len", 2);
	mock().expectOneCall("on_strchunk")
		.withStringParameter("chunk", &str[6])
		.withParameter("chunk_len", 3);
	LONGS_EQUAL(3, strchunk(str, delimiter, on_strchunk, NULL));
}
