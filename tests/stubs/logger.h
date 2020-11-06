#ifndef LOGGER_H
#define LOGGER_H 1

#define COMBINE_XY(x, y)		x##y
#define COMBINE(x, y)			COMBINE_XY(x, y)
#define get_program_counter()		({ \
		COMBINE(pos, __LINE__): \
		(void *)((volatile uintptr_t)&&COMBINE(pos, __LINE__)); \
})

#define LOG_DECORATOR(type, ...)	({ \
		test_logger(type, \
				get_program_counter(), \
				__builtin_return_address(0), \
				__VA_ARGS__); \
})

#define verbose(...) \
	LOG_DECORATOR(0, __VA_ARGS__)
#define debug(...) \
	LOG_DECORATOR(1, __VA_ARGS__)
#define info(...) \
	LOG_DECORATOR(2, __VA_ARGS__)
#define notice(...) \
	LOG_DECORATOR(3, __VA_ARGS__)
#define warn(...) \
	LOG_DECORATOR(4, __VA_ARGS__)
#define error(...) \
	LOG_DECORATOR(5, __VA_ARGS__)
#define alert(...) \
	LOG_DECORATOR(6, __VA_ARGS__)
#define whisper()			alert("")

#define logger_read(buf, bufsize)
#define logger_peek(buf, bufsize)
#define logger_consume(size)
#define logger_count()
#define logger_init()

#include <stdio.h>
#include <stdarg.h>

static inline int test_logger(const int type, void *pc, void *lr, ...)
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
	return len;
}

#endif /* LOGGER_H */
