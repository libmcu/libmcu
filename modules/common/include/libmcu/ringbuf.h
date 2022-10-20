/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_RINGBUF_H
#define LIBMCU_RINGBUF_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "libmcu/compiler.h"

struct ringbuf {
	size_t capacity;
	size_t index;
	size_t outdex;
	uint8_t *buffer;
};

#define DEFINE_RINGBUF(_name, _buf, _bufsize) \
	struct ringbuf _name = { \
		.capacity = _bufsize, \
		.index = 0, \
		.outdex = 0, \
		.buffer = _buf, \
	} \
	LIBMCU_STATIC_ASSERT(_bufsize & (_bufsize - 1) == 0, \
			      "_bufsize should be power of 2.");

size_t ringbuf_write(struct ringbuf *handle, const void *data, size_t datasize);
size_t ringbuf_write_cancel(struct ringbuf *handle, size_t size);
size_t ringbuf_peek(const struct ringbuf *handle,
		size_t offset, void *buf, size_t bufsize);
bool ringbuf_consume(struct ringbuf *handle, size_t consume_size);
size_t ringbuf_read(struct ringbuf *handle,
		size_t offset, void *buf, size_t bufsize);
size_t ringbuf_length(const struct ringbuf *handle);
size_t ringbuf_capacity(const struct ringbuf *handle);

bool ringbuf_create_static(struct ringbuf *handle, void *buf, size_t bufsize);
struct ringbuf *ringbuf_create(size_t space_size);
void ringbuf_destroy(struct ringbuf *handle);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RINGBUF_H */
