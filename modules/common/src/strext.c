/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/strext.h"
#include <string.h>
#include <stddef.h>

char *strtrim(char *s, const char c)
{
	const size_t len = strlen(s);
	size_t i = len? len - 1 : 0;

	while (s[i] == c) {
		s[i] = '\0';

		if (i == 0) {
			break;
		}

		i -= 1;
	}

	for (i = 0; i < len; i++) {
		if (s[i] != c) {
			break;
		}
	}

	if (i > 0) {
		for (size_t j = 0; j < len - i; j++) {
			s[j] = s[i+j];
		}
		s[len-i] = '\0';
	}

	return s;
}
