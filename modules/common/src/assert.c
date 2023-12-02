/*
 * SPDX-FileCopyrightText: 2021 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/assert.h"
#include "libmcu/logging.h"

LIBMCU_WEAK
LIBMCU_NORETURN
void libmcu_assertion_failed(const uintptr_t *pc, const uintptr_t *lr)
{
	error("Assertion at %p from %p", pc, lr);
	while (1) { /* hang */ }
}
