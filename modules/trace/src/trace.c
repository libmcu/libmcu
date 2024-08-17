/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/trace.h"
#include <stdatomic.h>

#define GET_INDEX(i, n)			((i) & ((n) - 1))
#if !defined(MIN)
#define MIN(a, b)			(((a) > (b))? (b) : (a))
#endif

static_assert((TRACE_MAXLEN & (TRACE_MAXLEN - 1)) == 0, "");

static struct {
	struct trace tracing[TRACE_MAXLEN];
	atomic_uint_least8_t call_depth;
	atomic_uint_least16_t index;
	atomic_uint_least16_t outdex;
} m;

LIBMCU_NO_INSTRUMENT
void trace_reset(void)
{
	atomic_store(&m.call_depth, 0);
	atomic_store(&m.index, 0);
	atomic_store(&m.outdex, 0);
}

LIBMCU_NO_INSTRUMENT
void trace_clear(void)
{
	atomic_store(&m.index, 0);
	atomic_store(&m.outdex, 0);
}

LIBMCU_NO_INSTRUMENT
size_t trace_count(void)
{
	uint16_t start = atomic_load(&m.outdex);
	uint16_t end = atomic_load(&m.index);

	return (size_t)MIN(end - start, TRACE_MAXLEN);
}

LIBMCU_NO_INSTRUMENT
void trace_iterate(trace_callback_t callback, void *ctx, int maxlen)
{
	uint16_t start = atomic_load(&m.outdex);
	uint16_t end = atomic_load(&m.index);
	unsigned int len = (unsigned int)maxlen;

	for (unsigned int i = 0; i < len && start != end; i++, start++) {
		if (callback) {
			uint16_t index = GET_INDEX(start, TRACE_MAXLEN);
			(*callback)(&m.tracing[index], ctx);
		}
	}
}

LIBMCU_NO_INSTRUMENT
void __cyg_profile_func_enter(void *callee, void *caller)
{
	uint8_t call_depth = atomic_fetch_add(&m.call_depth, 1);
	uint16_t index = GET_INDEX(atomic_fetch_add(&m.index, 1), TRACE_MAXLEN);
	struct trace *entry = &m.tracing[index];

	entry->callee = callee;
	entry->caller = caller;
	entry->depth = call_depth;
	entry->thread = trace_get_current_thread();
	entry->timestamp = trace_get_time();
	entry->stack_usage = trace_get_stack_watermark();

	trace_enter_hook(entry);

	unused(caller);
}

LIBMCU_NO_INSTRUMENT
void __cyg_profile_func_exit(void *callee, void *caller)
{
	struct trace entry = {
		.callee = callee,
		.caller = caller,
		.depth = atomic_load(&m.call_depth),
	};

	trace_leave_hook(&entry);

	atomic_fetch_sub(&m.call_depth, 1);

	unused(callee);
	unused(caller);
}
