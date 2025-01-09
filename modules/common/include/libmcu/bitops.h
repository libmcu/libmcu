/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
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

#if !defined(_STRING_H_)
int flsl(long x);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BITOPS_H */
