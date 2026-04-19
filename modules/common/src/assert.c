/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/assert.h"

LIBMCU_WEAK
LIBMCU_NORETURN
void libmcu_assertion_failed(const uintptr_t *pc, const uintptr_t *lr)
{
	unused(pc);
	unused(lr);
	while (1) { /* hang */ }
}
