/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

#include "libmcu/strext.h"

TEST_GROUP(STREXT) {
	void setup(void) {
	}
	void teardown(void) {
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
