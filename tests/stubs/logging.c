#include "libmcu/logging.h"
#include <stdio.h>
#include <stdarg.h>

size_t logging_save(const logging_t type, const void *pc, const void *lr, ...)
{
	int len = 0;
	va_list ap;
	const char *fmt;
	va_start(ap, lr);
	fmt = va_arg(ap, char *);
	if (fmt) {
		len = printf("[%s]\t", type == 0? "VERBOSE" :
				type == 1? "DEBUG" :
				type == 2? "INFO" :
				type == 3? "NOTICE" :
				type == 4? "WARN" :
				type == 5? "ERROR" :
				type == 6? "ALERT" : "UNKNOWN");
		len += vfprintf(stdout, fmt, ap);
		len += printf(" pc@%p, lr@%p\n", pc, lr);
	}
	return (size_t)len;
}
