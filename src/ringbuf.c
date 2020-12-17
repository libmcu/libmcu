#include "libmcu/ringbuf.h"
#include <string.h>

static inline size_t space_capacity(const ringbuf_t *self)
{
	return self->total_space;
}

static inline size_t space_used(const ringbuf_t *self)
{
	return self->space_used;
}

static inline size_t space_available(const ringbuf_t *self)
{
	return space_capacity(self) - space_used(self);
}

size_t ringbuf_write(ringbuf_t *self, const void *data, size_t data_size)
{
	if (space_available(self) < data_size) {
		return 0;
	}

	size_t write_offset = (self->read_offset + self->space_used)
		% self->total_space;
	size_t bytes_contiguous = self->total_space - write_offset;
	size_t bytes_remained = (bytes_contiguous < data_size)?
		data_size - bytes_contiguous : 0;

	memcpy(self->space + write_offset, data, data_size - bytes_remained);
	memcpy(self->space, ((const uint8_t *)data + data_size - bytes_remained),
			bytes_remained);

	self->space_used += data_size;

	return data_size;
}

size_t ringbuf_write_cancel(ringbuf_t *self, size_t size)
{
	if (space_used(self) < size) {
		return 0;
	}

	self->space_used -= size;
	return size;
}

size_t ringbuf_read(const ringbuf_t *self,
		size_t offset, void *buf, size_t data_size)
{
	if (self->space_used < data_size + offset) {
		return 0;
	}

	size_t read_offset = (self->read_offset + offset) % self->total_space;
	size_t bytes_contiguous = self->total_space - read_offset;
	size_t bytes_remained = (bytes_contiguous < data_size)?
		data_size - bytes_contiguous : 0;
	size_t bytes_to_read = data_size - bytes_remained;

	memcpy(buf, &self->space[read_offset], bytes_to_read);
	memcpy((uint8_t *)buf + bytes_to_read, self->space, bytes_remained);

	return data_size;
}

bool ringbuf_consume(ringbuf_t *self, size_t consume_size)
{
	if (self->space_used < consume_size) {
		return false;
	}

	self->read_offset = (self->read_offset + consume_size)
		% self->total_space;
	self->space_used -= consume_size;

	return true;
}

size_t ringbuf_used(const ringbuf_t *self)
{
	return space_used(self);
}

size_t ringbuf_left(const ringbuf_t *self)
{
	return space_available(self);
}

bool ringbuf_init(ringbuf_t *self, void *space, size_t space_size)
{
	if (!self || !space) {
		return false;
	}

	memset(space, 0, space_size);

	self->total_space = space_size;
	self->space_used = 0;
	self->read_offset = 0;
	self->space = space;

	return true;
}

#include <stdlib.h>

ringbuf_t *ringbuf_new(size_t space_size)
{
	ringbuf_t *ringbuf;

	if (!(ringbuf = calloc(1, sizeof(*ringbuf)))) {
		return NULL;
	}
	if (!(ringbuf->space = calloc(1, space_size))) {
		free(ringbuf);
		return NULL;
	}

	ringbuf_init(ringbuf, ringbuf->space, space_size);

	return ringbuf;
}

void ringbuf_delete(ringbuf_t *self)
{
	free(self->space);
	free(self);
}
