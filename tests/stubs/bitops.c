/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/bitops.h"

int flsl(long x)
{
	return x? (int)sizeof(x) * CHAR_BIT - __builtin_clzl((unsigned long)x) : 0;
}
