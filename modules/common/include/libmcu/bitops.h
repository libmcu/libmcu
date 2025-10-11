/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BITOPS_H
#define LIBMCU_BITOPS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

int flsl(long x);

static inline bool is_power2(const unsigned long x)
{
	return (x != 0) && ((x & (x - 1)) == 0);
}

/**
 * @brief Rounds up a value to the next power of 2.
 *
 * If the value is already a power of 2, it returns the value unchanged.
 * If the value is 0, it returns 1.
 *
 * @param[in] x The value to round up.
 *
 * @return The smallest power of 2 that is greater than or equal to x.
 */
static inline size_t roundup_pow2(size_t x)
{
	if (x == 0) {
		return 1;
	} else if (is_power2((unsigned long)x)) {
		return x;
	}
	return (size_t)(1UL << flsl((long)x));
}

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BITOPS_H */
