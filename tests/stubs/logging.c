#include "libmcu/logging.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

size_t logging_save(const logging_t type, const void *pc, const void *lr, ...)
{
	int len = printf("%lu: [%s] <%p,%p> ", (unsigned long)time(0),
			type == 0? "VERBOSE" :
			type == 1? "DEBUG" :
			type == 2? "INFO" :
			type == 3? "NOTICE" :
			type == 4? "WARN" :
			type == 5? "ERROR" :
			type == 6? "ALERT" : "UNKNOWN",
			pc, lr);
	va_list ap;
	va_start(ap, lr);
	const char *fmt = va_arg(ap, char *);
	if (fmt) {
		len += vfprintf(stdout, fmt, ap);
		len += printf("\n");
	}
	return (size_t)len;
}
