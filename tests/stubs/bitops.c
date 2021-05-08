#include "libmcu/bitops.h"

int flsl(long x)
{
	return x? (int)sizeof(x) * CHAR_BIT - __builtin_clzl((unsigned long)x) : 0;
}
