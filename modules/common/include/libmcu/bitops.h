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
#include <limits.h>

static inline bool is_power2(const unsigned long x)
{
	return (x != 0) && ((x & (x - 1)) == 0);
}

int flsl(long x);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BITOPS_H */
