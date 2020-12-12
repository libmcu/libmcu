#include "ringbuf.h"
#include <string.h>

static inline size_t space_capacity(const ringbuf_t *obj)
{
	return obj->total_space;
}

static inline size_t space_used(const ringbuf_t *obj)
{
	return obj->space_used;
}

static inline size_t space_available(const ringbuf_t *obj)
{
	return space_capacity(obj) - space_used(obj);
}

size_t ringbuf_write(ringbuf_t *obj, const void *data, size_t data_size)
{
	if (space_available(obj) < data_size) {
		return 0;
	}

	size_t write_offset = (obj->read_offset + obj->space_used) % obj->total_space;
	size_t bytes_contiguous = obj->total_space - write_offset;
	size_t bytes_remained = (bytes_contiguous < data_size)?
		data_size - bytes_contiguous : 0;

	memcpy(obj->space + write_offset, data, data_size - bytes_remained);
	memcpy(obj->space, ((const uint8_t *)data + data_size - bytes_remained),
			bytes_remained);

	obj->space_used += data_size;

	return data_size;
}

size_t ringbuf_write_cancel(ringbuf_t *obj, size_t size)
{
	if (space_used(obj) < size) {
		return 0;
	}

	obj->space_used -= size;
	return size;
}

size_t ringbuf_read(ringbuf_t *obj, size_t offset, void *buf, size_t data_size)
{
	if (obj->space_used < data_size + offset) {
		return 0;
	}

	size_t read_offset = (obj->read_offset + offset) % obj->total_space;
	size_t bytes_contiguous = obj->total_space - read_offset;
	size_t bytes_remained = (bytes_contiguous < data_size)?
		data_size - bytes_contiguous : 0;
	size_t bytes_to_read = data_size - bytes_remained;

	memcpy(buf, &obj->space[read_offset], bytes_to_read);
	memcpy((uint8_t *)buf + bytes_to_read, obj->space, bytes_remained);

	return data_size;
}

bool ringbuf_consume(ringbuf_t *obj, size_t consume_size)
{
	if (obj->space_used < consume_size) {
		return false;
	}

	obj->read_offset = (obj->read_offset + consume_size) % obj->total_space;
	obj->space_used -= consume_size;

	return true;
}

size_t ringbuf_used(ringbuf_t *obj)
{
	return space_used(obj);
}

size_t ringbuf_left(ringbuf_t *obj)
{
	return space_available(obj);
}

bool ringbuf_init(ringbuf_t *obj, void *space, size_t space_size)
{
	if (!obj || !space) {
		return false;
	}

	memset(space, 0, space_size);

	obj->total_space = space_size;
	obj->space_used = 0;
	obj->read_offset = 0;
	obj->space = space;

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

void ringbuf_delete(ringbuf_t *obj)
{
	free(obj->space);
	free(obj);
}
