/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_FAULT_H
#define LIBMCU_FAULT_H

#if defined(__cplusplus)
extern "C" {
#endif

#define FAULT_ENTER()	\
	__asm__ __volatile__(					\
			"tst	lr, #4			\n\t"	\
			"ite	eq			\n\t"	\
			"mrseq	r3, msp			\n\t"	\
			"mrsne	r3, psp			\n\t"	\
			"push	{r3-r11, lr}		\n\t"	\
			"mov	r0, sp			\n\t"	\
			"b	fault_process		\n\t"	\
			::: "memory")

struct core_registers;
void fault_process(struct core_registers *regs);
int fault_save(const struct core_registers *regs);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_FAULT_H */
