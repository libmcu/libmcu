/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/assert.h"
#include "libmcu/logging.h"

void libmcu_assertion_failed(const uintptr_t *pc, const uintptr_t *lr)
{
	error("Assertion at %p from %p", pc, lr);
	__asm__ __volatile__("bkpt 41"); /* 46 represents 'A' in ascii */

	/* reset */
	volatile uint32_t *aircr = (volatile uint32_t *)0xE000ED0C;
	*aircr = (0x05FA/*VECTKEY*/ << 16) | 0x1 << 2/*SYSRESETREQ*/;
}
