/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/logging.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

size_t logging_write(logging_t type, const struct logging_context *ctx, ...)
{
	int len = printf("%lu: [%s] <%p,%p> ", (unsigned long)time(0),
			type == 0? "NONE" :
			type == 1? "DEBUG" :
			type == 2? "INFO" :
			type == 3? "WARN" :
			type == 4? "ERROR" : "UNKNOWN",
			ctx->pc, ctx->lr);
	va_list ap;
	va_start(ap, ctx);
	const char *fmt = va_arg(ap, char *);
	if (fmt) {
		len += vfprintf(stdout, fmt, ap);
		len += printf("\n");
	}
	return (size_t)len;
}
