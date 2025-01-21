/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/strext.h"
#include <string.h>
#include <stddef.h>

size_t strchunk(const char *str, const char delimiter,
		strchunk_cb_t cb, void *cb_ctx)
{
	const char *start = str;
	const char *end = str;
	size_t count = 0;

	while (*end) {
		const size_t len = (size_t)(end - start);
		if (*end == delimiter) {
			if (cb && len) {
				(*cb)(start, len, cb_ctx);
			}
			start = end + 1;
			count = len? count + 1 : count;
		}
		end++;
	}

	if (start != end) {
		if (cb) {
			(*cb)(start, (size_t)(end - start), cb_ctx);
		}
		count++;
	}

	return count;
}

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

void strupper(char *s)
{
	while (s && *s) {
		if (*s >= 'a' && *s <= 'z') {
			*s = *s - 'a' + 'A';
		}
		s++;
	}
}

void strlower(char *s)
{
	while (s && *s) {
		if (*s >= 'A' && *s <= 'Z') {
			*s = *s - 'A' + 'a';
		}
		s++;
	}
}
