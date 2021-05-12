#include "libmcu/ringbuf.h"
#include <string.h>
#include "libmcu/bitops.h"

#define GET_INDEX(i, n)			((i) & ((n) - 1))

static inline size_t get_capacity(const ringbuf_t handle)
{
	return handle->capacity;
}

static inline size_t get_length(const ringbuf_t handle)
{
	return handle->index - handle->outdex;
}

static inline size_t get_available(const ringbuf_t handle)
{
	return get_capacity(handle) - get_length(handle);
}

size_t ringbuf_write(ringbuf_t handle, const void *data, size_t data_size)
{
	if (get_available(handle) < data_size) {
		return 0;
	}

	size_t index = GET_INDEX(handle->index, handle->capacity);
	size_t contiguous = handle->capacity - index;
	size_t remained = (contiguous < data_size)? data_size - contiguous : 0;

	memcpy(handle->buffer + index, data, data_size - remained);
	memcpy(handle->buffer, ((const uint8_t *)data + data_size - remained),
			remained);

	handle->index += data_size;

	return data_size;
}

size_t ringbuf_write_cancel(ringbuf_t handle, size_t size)
{
	if (get_length(handle) < size) {
		return 0;
	}

	handle->index -= size;

	return size;
}

size_t ringbuf_read(const ringbuf_t handle,
		size_t offset, void *buf, size_t data_size)
{
	if (get_length(handle) < data_size + offset) {
		return 0;
	}

	size_t index = GET_INDEX(handle->outdex + offset, handle->capacity);
	size_t contiguous = handle->capacity - index;
	size_t remained = (contiguous < data_size)? data_size - contiguous : 0;
	size_t cut = data_size - remained;

	memcpy(buf, &handle->buffer[index], cut);
	memcpy((uint8_t *)buf + cut, handle->buffer, remained);

	return data_size;
}

bool ringbuf_consume(ringbuf_t handle, size_t consume_size)
{
	if (get_length(handle) < consume_size) {
		return false;
	}

	handle->outdex += consume_size;

	return true;
}

size_t ringbuf_length(const ringbuf_t handle)
{
	return get_length(handle);
}

size_t ringbuf_capacity(const ringbuf_t handle)
{
	return get_capacity(handle);
}

bool ringbuf_create_static(ringbuf_static_t *instance, void *buf, size_t bufsize)
{
	if (instance == NULL || buf == NULL || bufsize < 2) {
		return false;
	}

	memset(buf, 0, bufsize);

	instance->capacity = 1U << (flsl((long)bufsize) - 1); // must be power of 2
	instance->index = 0;
	instance->outdex = 0;
	instance->buffer = (uint8_t *)buf;

	return true;
}

#include <stdlib.h>

ringbuf_t ringbuf_create(size_t space_size)
{
	ringbuf_t ringbuf;

	if (!(ringbuf = (ringbuf_t)calloc(1, sizeof(*ringbuf)))) {
		return NULL;
	}
	if (!(ringbuf->buffer = (uint8_t *)calloc(1, space_size))) {
		free(ringbuf);
		return NULL;
	}

	ringbuf_create_static(ringbuf, ringbuf->buffer, space_size);

	return ringbuf;
}

void ringbuf_destroy(ringbuf_t handle)
{
	free(handle->buffer);
	free(handle);
}
