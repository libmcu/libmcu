/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/timext.h"
#include <string.h>
#include <stdint.h>

static void strcpy_iso8601_removing_unsupported(char *buf, size_t bufsize,
		const char *str)
{
	char *p, *t;

	strncpy(buf, str, bufsize);

	if ((p = strstr(buf, ".")) == NULL) {
		return;
	}

	/* remove milli seconds */
	if ((t = strstr(p, "+")) != NULL || (t = strstr(p, "-")) != NULL) {
		for (int i = 0; t[i]; i++) {
			p[i] = t[i];
		}
	}
}

static int get_time_difference_from_timezone(const char *str)
{
	const size_t len = strlen(str);
	char *p, *t;

	/* find time string */
	if ((t = strstr(str, "T")) == NULL) {
		return 0;
	}

	if ((p = strstr(t, "+")) == NULL && (p = strstr(t, "-")) == NULL) {
		return 0;
	}

	const size_t tzlen = len - (size_t)((uintptr_t)p - (uintptr_t)str);
	const int hour = (p[1] - '0') * 10 + (p[2] - '0');
	int min = 0;

	if (tzlen > 5 && strstr(p, ":") != NULL) {
		min = (p[4] - '0') * 10 + (p[5] - '0');
	} else if (tzlen == 5) {
		min = (p[3] - '0') * 10 + (p[4] - '0');
	}

	const int seconds = (hour * 60 + min) * 60 * (p[0] == '-'? 1 : -1);

	return seconds;
}

time_t iso8601_convert_to_time(const char *tstr)
{
	struct tm tim;
	time_t t = -1; /* time_t never gets negative value. */
	char tmp[32] = { 0, };

	strcpy_iso8601_removing_unsupported(tmp, sizeof(tmp)-1, tstr);

	if (strptime(tmp, "%Y-%m-%dT%H:%M:%S%z", &tim) != NULL) {
		t = mktime(&tim);
	} else if (strptime(tmp, "%Y-%m-%dT%H:%M:%S", &tim) != NULL) {
		t = mktime(&tim);
		t += get_time_difference_from_timezone(tmp);
	}

	return t;
}
