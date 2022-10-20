/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/trace.h"
#include <string.h>
#include "libmcu/ringbuf.h"
#include "libmcu/compiler.h"

DEFINE_RINGBUF(trace_buffer, TRACE_BUFSIZE);
static uint8_t call_depth;

LIBMCU_NO_INSTRUMENT
void trace_reset(void)
{
	ringbuf_consume(&trace_buffer, ringbuf_length(&trace_buffer));
	call_depth = 0;
}

LIBMCU_NO_INSTRUMENT
void trace_clear(void)
{
	ringbuf_consume(&trace_buffer, ringbuf_length(&trace_buffer));
}

LIBMCU_NO_INSTRUMENT
size_t trace_count(void)
{
	return ringbuf_length(&trace_buffer) / sizeof(struct trace);
}

LIBMCU_NO_INSTRUMENT
size_t trace_read(void *buf, size_t bufsize)
{
	unused(buf);
	unused(bufsize);
	return 0;
}

LIBMCU_NO_INSTRUMENT
void trace_iterate(trace_callback_t callback, void *ctx)
{
	struct trace entry;
	size_t offset = 0;
	size_t bytes_read;

	while ((bytes_read = ringbuf_read(&trace_buffer, offset,
				&entry, sizeof(entry))) >= sizeof(entry)) {
		if (callback) {
			(*callback)(&entry, ctx);
		}

		offset += bytes_read;
	}
}

LIBMCU_NO_INSTRUMENT
void __cyg_profile_func_enter(void *callee, void *caller)
{
	call_depth++;

	if (caller == __cyg_profile_func_enter ||
			caller == trace_iterate ||
			caller == trace_read ||
			caller == trace_count ||
			caller == trace_clear ||
			caller == trace_reset) {
		return;
	}

	struct trace entry = {
		.callee = callee,
		.depth = call_depth,
	};
	ringbuf_write(&trace_buffer, &entry, sizeof(entry));
}

LIBMCU_NO_INSTRUMENT
void __cyg_profile_func_exit(void *callee, void *caller)
{
	unused(callee);
	unused(caller);

	call_depth--;
}
