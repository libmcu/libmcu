/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_TRACE_H
#define LIBMCU_TRACE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#if !defined(TRACE_BUFSIZE)
#define TRACE_BUFSIZE			1024
#endif

struct trace {
	uint32_t timestamp;
	void *callee;
	uint8_t depth;
};

typedef void (*trace_callback_t)(const struct trace *entry, void *ctx);

/**
 * @brief Reset the call depth and clear the buffer
 */
void trace_reset(void);
/**
 * @brief Clear trace buffer
 * @note This does not reset the call depth.
 */
void trace_clear(void);
/**
 * @brief Count the number of tracing recorded
 * @return The number of tracing
 */
size_t trace_count(void);
size_t trace_read(void *buf, size_t bufsize);
void trace_iterate(trace_callback_t callback, void *ctx);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_TRACE_H */
