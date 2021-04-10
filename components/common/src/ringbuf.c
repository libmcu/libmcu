#include "libmcu/ringbuf.h"
#include <string.h>
#include "libmcu/bitops.h"

#define GET_INDEX(i, n)			((i) & ((n) - 1))

static inline size_t get_capacity(const ringbuf_t *self)
{
	return self->capacity;
}

static inline size_t get_length(const ringbuf_t *self)
{
	return self->index - self->outdex;
}

static inline size_t get_available(const ringbuf_t *self)
{
	return get_capacity(self) - get_length(self);
}

size_t ringbuf_write(ringbuf_t *self, const void *data, size_t data_size)
{
	if (get_available(self) < data_size) {
		return 0;
	}

	size_t index = GET_INDEX(self->index, self->capacity);
	size_t contiguous = self->capacity - index;
	size_t remained = (contiguous < data_size)? data_size - contiguous : 0;

	memcpy(self->buffer + index, data, data_size - remained);
	memcpy(self->buffer, ((const uint8_t *)data + data_size - remained),
			remained);

	self->index += data_size;

	return data_size;
}

size_t ringbuf_write_cancel(ringbuf_t *self, size_t size)
{
	if (get_length(self) < size) {
		return 0;
	}

	self->index -= size;

	return size;
}

size_t ringbuf_read(const ringbuf_t *self,
		size_t offset, void *buf, size_t data_size)
{
	if (get_length(self) < data_size + offset) {
		return 0;
	}

	size_t index = GET_INDEX(self->outdex + offset, self->capacity);
	size_t contiguous = self->capacity - index;
	size_t remained = (contiguous < data_size)? data_size - contiguous : 0;
	size_t cut = data_size - remained;

	memcpy(buf, &self->buffer[index], cut);
	memcpy((uint8_t *)buf + cut, self->buffer, remained);

	return data_size;
}

bool ringbuf_consume(ringbuf_t *self, size_t consume_size)
{
	if (get_length(self) < consume_size) {
		return false;
	}

	self->outdex += consume_size;

	return true;
}

size_t ringbuf_length(const ringbuf_t *self)
{
	return get_length(self);
}

size_t ringbuf_capacity(const ringbuf_t *self)
{
	return get_capacity(self);
}

bool ringbuf_init(ringbuf_t *self, void *buf, size_t bufsize)
{
	if (self == NULL || buf == NULL || bufsize < 2) {
		return false;
	}

	memset(buf, 0, bufsize);

	self->capacity = 1U << (flsl((long)bufsize) - 1); // must be power of 2
	self->index = 0;
	self->outdex = 0;
	self->buffer = (uint8_t *)buf;

	return true;
}

#include <stdlib.h>

ringbuf_t *ringbuf_new(size_t space_size)
{
	ringbuf_t *ringbuf;

	if (!(ringbuf = (ringbuf_t *)calloc(1, sizeof(*ringbuf)))) {
		return NULL;
	}
	if (!(ringbuf->buffer = (uint8_t *)calloc(1, space_size))) {
		free(ringbuf);
		return NULL;
	}

	ringbuf_init(ringbuf, ringbuf->buffer, space_size);

	return ringbuf;
}

void ringbuf_delete(ringbuf_t *self)
{
	free(self->buffer);
	free(self);
}
