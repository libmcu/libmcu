/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/ringbuf.h"

#include <string.h>
#include <stdlib.h>
#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
#include <stdatomic.h>
#endif

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

static void initialize(struct ringbuf *handle, const size_t bufsize)
{
	handle->capacity = 1U << (flsl((long)bufsize) - 1); /* should be
							       power of 2 */
	handle->index = 0;
	handle->outdex = 0;
}

static uint8_t *get_pointer(const struct ringbuf *handle,
		const size_t offset, size_t *contiguous)
{
	size_t index = GET_INDEX(handle->outdex + offset, handle->capacity);
	uint8_t *p = &handle->buffer[index];

	if (offset >= get_length(handle)) {
		p = NULL;
		index = handle->capacity;
	}

	if (contiguous) {
		*contiguous = handle->capacity - index;
	}

	return p;
}

static size_t read_core(const struct ringbuf *handle,
		const size_t offset, void *buf, const size_t bufsize)
{
	size_t contiguous;
	const uint8_t *p = get_pointer(handle, offset, &contiguous);
	const uint8_t *base = handle->buffer;
	size_t bytes_read = 0;

	if (p) {
		const size_t len = MIN(get_length(handle) - offset, bufsize);
		const size_t remained = contiguous < len? len - contiguous : 0;
		const size_t cut = len - remained;

		memcpy(buf, p, cut);
		memcpy((uint8_t *)buf + cut, base, remained);

		bytes_read = len;
	}

	return bytes_read;
}

static bool consume_core(struct ringbuf *handle, const size_t consume_size)
{
	if (get_length(handle) < consume_size) {
		return false;
	}

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
	atomic_thread_fence(memory_order_acquire);
#endif
	handle->outdex += consume_size;

	return true;
}

size_t ringbuf_write(struct ringbuf *handle,
		const void *data, const size_t datasize)
{
	const size_t len = MIN(get_available(handle), datasize);
	const size_t index = GET_INDEX(handle->index, handle->capacity);
	const size_t contiguous = handle->capacity - index;
	const size_t remained = (contiguous < len)? len - contiguous : 0;
	const size_t cut = len - remained;

	memcpy(handle->buffer + index, data, cut);
	memcpy(handle->buffer, ((const uint8_t *)data + cut), remained);

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
	atomic_thread_fence(memory_order_release);
#endif
	handle->index += len;

	return len;
}

size_t ringbuf_write_cancel(struct ringbuf *handle, const size_t size)
{
	if (get_length(handle) < size) {
		return 0;
	}

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
	atomic_thread_fence(memory_order_release);
#endif
	handle->index -= size;

	return size;
}

size_t ringbuf_peek(const struct ringbuf *handle,
		const size_t offset, void *buf, const size_t bufsize)
{
	return read_core(handle, offset, buf, bufsize);
}

const void *ringbuf_peek_pointer(const struct ringbuf *handle,
		const size_t offset, size_t *contiguous)
{
	const void *p = get_pointer(handle, offset, contiguous);

	if (contiguous) {
		*contiguous = MIN(*contiguous, get_length(handle) - offset);
	}

	return p;
}

bool ringbuf_consume(struct ringbuf *handle, const size_t consume_size)
{
	return consume_core(handle, consume_size);
}

size_t ringbuf_read(struct ringbuf *handle,
		const size_t offset, void *buf, const size_t bufsize)
{
	size_t bytes_read = read_core(handle, offset, buf, bufsize);

	if (bytes_read > 0) {
		consume_core(handle, bytes_read + offset);
	}

	return bytes_read;
}

size_t ringbuf_length(const struct ringbuf *handle)
{
	return get_length(handle);
}

size_t ringbuf_capacity(const struct ringbuf *handle)
{
	return get_capacity(handle);
}

bool ringbuf_create_static(struct ringbuf *handle,
		void *buf, const size_t bufsize)
{
	if (handle == NULL || buf == NULL || bufsize == 0) {
		return false;
	}

	handle->buffer = (uint8_t *)buf;
	initialize(handle, bufsize);

	return true;
}

struct ringbuf *ringbuf_create(const size_t space_size)
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
