/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ringbuf.h"

#include <string.h>
#include <stdlib.h>

#include "libmcu/compiler.h"
#include "libmcu/bitops.h"

#define GET_INDEX(i, n)			((i) & ((n) - 1))
#if !defined(MIN)
#define MIN(a, b)			(((a) > (b)) ? (b) : (a))
#endif

static size_t get_capacity(const struct ringbuf *handle)
{
	return handle->capacity;
}

static size_t get_length(const struct ringbuf *handle)
{
	return handle->index - handle->outdex;
}

static size_t get_available(const struct ringbuf *handle)
{
	return get_capacity(handle) - get_length(handle);
}

static void initialize(struct ringbuf *handle, size_t bufsize)
{
	memset(handle->buffer, 0, bufsize);

	handle->capacity = 1U << (flsl((long)bufsize) - 1); /* should be
							       power of 2 */
	handle->index = 0;
	handle->outdex = 0;
}

static size_t read_core(const struct ringbuf *handle,
		size_t offset, void *buf, size_t bufsize)
{
	bufsize = MIN(get_length(handle), bufsize);

	size_t index = GET_INDEX(handle->outdex + offset, handle->capacity);
	size_t contiguous = handle->capacity - index;
	size_t remained = (contiguous < bufsize)? bufsize - contiguous : 0;
	size_t cut = bufsize - remained;

	memcpy(buf, &handle->buffer[index], cut);
	memcpy((uint8_t *)buf + cut, handle->buffer, remained);

	return bufsize;
}

static bool consume_core(struct ringbuf *handle, size_t consume_size)
{
	if (get_length(handle) < consume_size) {
		return false;
	}

	handle->outdex += consume_size;

	return true;
}

size_t ringbuf_write(struct ringbuf *handle, const void *data, size_t datasize)
{
	const size_t n = MIN(get_available(handle), datasize);
	const size_t index = GET_INDEX(handle->index, handle->capacity);
	const size_t contiguous = handle->capacity - index;
	const size_t remained = (contiguous < n)? n - contiguous : 0;

	memcpy(handle->buffer + index, data, n - remained);
	memcpy(handle->buffer,
			((const uint8_t *)data + n - remained), remained);

	handle->index += n;

	return n;
}

size_t ringbuf_write_cancel(struct ringbuf *handle, size_t size)
{
	if (get_length(handle) < size) {
		return 0;
	}

	handle->index -= size;

	return size;
}

size_t ringbuf_peek(const struct ringbuf *handle,
		size_t offset, void *buf, size_t bufsize)
{
	return read_core(handle, offset, buf, bufsize);
}

bool ringbuf_consume(struct ringbuf *handle, size_t consume_size)
{
	return consume_core(handle, consume_size);
}

size_t ringbuf_read(struct ringbuf *handle,
		size_t offset, void *buf, size_t bufsize)
{
	size_t bytes_read = read_core(handle, offset, buf, bufsize);

	if (bytes_read > 0) {
		consume_core(handle, bytes_read);
	}

	return bytes_read;
}

size_t ringbuf_length(const struct ringbuf *handle)
{
	return get_length((const struct ringbuf *)handle);
}

size_t ringbuf_capacity(const struct ringbuf *handle)
{
	return get_capacity((const struct ringbuf *)handle);
}

bool ringbuf_create_static(struct ringbuf *handle, void *buf, size_t bufsize)
{
	if (handle == NULL || buf == NULL || bufsize == 0) {
		return false;
	}

	handle->buffer = (uint8_t *)buf;
	initialize(handle, bufsize);

	return true;
}

struct ringbuf *ringbuf_create(size_t space_size)
{
	struct ringbuf *instance = NULL;

	if (space_size == 0) {
		return NULL;
	}
	if (!(instance = (struct ringbuf *)calloc(1, sizeof(*instance)))) {
		return NULL;
	}
	if (!(instance->buffer = (uint8_t *)calloc(1, space_size))) {
		free(instance);
		return NULL;
	}

	initialize(instance, space_size);

	return instance;
}

void ringbuf_destroy(struct ringbuf *handle)
{
	if (handle == NULL) {
		return;
	}

	free(handle->buffer);
	free(handle);
}
