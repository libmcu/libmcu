/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/fault.h"

#include <stdint.h>
#include <stdbool.h>

#include "libmcu/compiler.h"
#include "libmcu/logging.h"

enum fault_type {
	USAGE_FAULT	= (1 << 3),
	BUS_FAULT	= (1 << 1),
	MM_FAULT	= (1 << 0),
};

enum usage_fault_status {
	UNDEFINSTR	= (1 << 16), /* Undefined instruction */
	INVSTATE	= (1 << 17), /* Invalid state of EPSR */
	INVPC		= (1 << 18), /* Invalid PC load by EXC_RETURN */
	NOCP		= (1 << 19), /* No coprocessor */
	UNALIGNED	= (1 << 24), /* Unaligned access */
	DIVBYZERO	= (1 << 25), /* Divide by zero */
};

enum bus_fault_status {
	IBUSERR		= (1 <<  8), /* Instruction bus error */
	PRECISERR	= (1 <<  9), /* Precise data bus error */
	IMPRECISERR	= (1 << 10), /* Imprecise data bus error */
	UNSTKERR	= (1 << 11), /* Fault on unstacking for a return from exception */
	STKERR		= (1 << 12), /* Fault on stacking for exception */
	BFARVALID	= (1 << 15), /* Address Register valid flag */
};

enum mm_fault_status {
	IACCVIOL	= (1 << 0), /* Instruction access violation flag */
	DACCVIOL	= (1 << 1), /* Data access violation flag */
	MUNSTKERR	= (1 << 3), /* Fault on unstacking for a return from exception */
	MSTKERR		= (1 << 4), /* Fault on stacking for exception */
	MMARVALID	= (1 << 7), /* Address register valid flag */
};

struct fault_exception_frame {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t xpsr;
} LIBMCU_PACKED;

struct core_registers {
	struct fault_exception_frame *exception_frame;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t exc_return;
} LIBMCU_PACKED;

static void print_fault(volatile uint32_t *cfsr)
{
	if (*cfsr & UNDEFINSTR) {
		error("Undefined instruction");
	}
	if (*cfsr & INVSTATE) {
		error("Invalid state of EPSR");
	}
	if (*cfsr & INVPC) {
		error("Invalid PC load by EXC_RETURN");
	}
	if (*cfsr & NOCP) {
		error("No coprocessor");
	}
	if (*cfsr & UNALIGNED) {
		error("Unalignd access");
	}
	if (*cfsr & DIVBYZERO) {
		error("Divide by zero");
	}
	if (*cfsr & IBUSERR) {
		error("Instruction bus error");
	}
	if (*cfsr & PRECISERR) {
		error("Precise data bus error");
	}
	if (*cfsr & IMPRECISERR) {
		error("Imprecise data bus error");
	}
	if (*cfsr & UNSTKERR) {
		error("Fault on unstacking for a return from exception");
	}
	if (*cfsr & STKERR) {
		error("Fault on stacking for exception");
	}
	if (*cfsr & BFARVALID) {
		volatile uint32_t *bfar = (volatile uint32_t *)0xE000ED38;
		error("\tFault address: 0x%08x", *bfar);
	}
	if (*cfsr & IACCVIOL) {
		error("Instruction access violation");
	}
	if (*cfsr & DACCVIOL) {
		error("Data access violation");
	}
	if (*cfsr & MUNSTKERR) {
		error("MM Fault on unstacking for a return from exception");
	}
	if (*cfsr & MSTKERR) {
		error("MM Fault on stacking for exception");
	}
	if (*cfsr & MMARVALID) {
		volatile uint32_t *mmfar = (volatile uint32_t *)0xE000ED34;
		error("\tMM fault address: 0x%08x", *mmfar);
	}
}

static void print_fault_registers(void)
{
	volatile uint32_t *shcsr = (volatile uint32_t *)0xE000ED24;
	volatile uint32_t *icsr = (volatile uint32_t *)0xE000ED04;
	volatile uint32_t *hfsr = (volatile uint32_t *)0xE000ED2C;
	volatile uint32_t *mmfar = (volatile uint32_t *)0xE000ED34;
	volatile uint32_t *bfar = (volatile uint32_t *)0xE000ED38;

	error("Exception registers");
	error("\tSHCSR 0x%08lx", *shcsr);
	error("\tICSR  0x%08lx", *icsr);
	error("\tHFSR  0x%08lx", *hfsr);
	error("\tMMFAR 0x%08lx", *mmfar);
	error("\tBFAR  0x%08lx", *bfar);
}

static void print_frame(struct core_registers *regs)
{
	error("Core registers");
	error("\tr0   0x%08lx", regs->exception_frame->r0);
	error("\tr1   0x%08lx", regs->exception_frame->r1);
	error("\tr2   0x%08lx", regs->exception_frame->r2);
	error("\tr3   0x%08lx", regs->exception_frame->r3);
	error("\tr4   0x%08lx", regs->r4);
	error("\tr5   0x%08lx", regs->r5);
	error("\tr6   0x%08lx", regs->r6);
	error("\tr7   0x%08lx", regs->r7);
	error("\tr8   0x%08lx", regs->r8);
	error("\tr9   0x%08lx", regs->r9);
	error("\tr10  0x%08lx", regs->r10);
	error("\tr11  0x%08lx", regs->r11);
	error("\tr12  0x%08lx", regs->exception_frame->r12);
	error("\tlr   0x%08lx", regs->exception_frame->lr);
	error("\tpc   0x%08lx", regs->exception_frame->pc);
	error("\txpsr 0x%08lx", regs->exception_frame->xpsr);
	error("\texc  0x%08lx", regs->exc_return);
}

static bool recover_from_fault(struct core_registers *regs)
{
	volatile uint32_t *cfsr = (volatile uint32_t *)0xE000ED28;
	const uint32_t usage_fault_mask = 0xffff0000;
	const bool non_usage_fault = (*cfsr & ~usage_fault_mask) != 0;

	const uint32_t nr_exception = regs->exception_frame->xpsr & 0xff;
	const bool fault_from_exception = nr_exception != 0;

	if (fault_from_exception || non_usage_fault) {
		return false;
	}

	*cfsr |= *cfsr;
	//regs->return_address = ;
	//regs->lr = ;
	regs->exception_frame->xpsr = (1 << 24)/*keep thumb bit*/;

	return true;
}

__attribute__((optimize("O0"), used, weak))
void fault_process(struct core_registers *regs)
{
	volatile uint32_t *dhcsr = (volatile uint32_t *)0xE000EDF0;
	const uint32_t c_debugen_mask = 0x1;
	const bool debugger_attached = !!(*dhcsr & c_debugen_mask);

	if (debugger_attached) {
		__asm__ __volatile__("bkpt 46"); /* 46 represents 'F' in ascii */
	}

	volatile uint32_t *cfsr = (volatile uint32_t *)0xE000ED28;

	if (*cfsr) {
		error("Fault (CFSR 0x%08lx)", *cfsr);

		print_fault(cfsr);
		print_fault_registers();
	} else {
		error("Assertion");
	}

	print_frame(regs);

	if (recover_from_fault(regs)) {
		return;
	}

	if (fault_save(regs) == 0) {
		/* reset */
		volatile uint32_t *aircr = (volatile uint32_t *)0xE000ED0C;
		*aircr = (0x05FA/*VECTKEY*/ << 16) | 0x1 << 2/*SYSRESETREQ*/;
	}

	while (1) {
		/* hang */
	}
}

LIBMCU_WEAK
int fault_save(const struct core_registers LIBMCU_UNUSED *regs)
{
	return 0;
}
