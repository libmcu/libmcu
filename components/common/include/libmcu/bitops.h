#ifndef LIBMCU_BITOPS_H
#define LIBMCU_BITOPS_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <limits.h>

#if !defined(__clang__)
static inline int flsl(long x)
{
	return x? (int)sizeof(x) * CHAR_BIT - __builtin_clzl((unsigned long)x) : 0;
}
#endif

static inline bool ispower2(unsigned int x)
{
	return (x & (x - 1)) == 0;
}

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BITOPS_H */
