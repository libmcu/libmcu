#ifndef LIBMCU_BITOPS_H
#define LIBMCU_BITOPS_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

static inline int fls(unsigned long x)
{
	return x? (int)sizeof(x) * 8 - __builtin_clzl(x) : 0;
}

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BITOPS_H */
