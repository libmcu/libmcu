#include "libmcu/ringbuf.h"
#include <string.h>
#include "libmcu/compiler.h"
#include "libmcu/bitops.h"

#define GET_INDEX(i, n)			((i) & ((n) - 1))

struct ringbuf {
	size_t capacity;
	size_t index;
	size_t outdex;
	uint8_t *buffer;
};
LIBMCU_STATIC_ASSERT(sizeof(struct ringbuf) == sizeof(ringbuf_static_t), "");

static inline size_t get_capacity(const struct ringbuf *instance)
{
	return instance->capacity;
}

static inline size_t get_length(const struct ringbuf *instance)
{
	return instance->index - instance->outdex;
}

static inline size_t get_available(const struct ringbuf *instance)
{
	return get_capacity(instance) - get_length(instance);
}

static void initialize_instance(struct ringbuf *instance, size_t bufsize)
{
	memset(instance->buffer, 0, bufsize);

	instance->capacity = 1U << (flsl((long)bufsize) - 1); /* should be
								 power of 2 */
	instance->index = 0;
	instance->outdex = 0;
}

static size_t read_core(const ringbuf_t *handle,
		size_t offset, void *buf, size_t data_size)
{
	const struct ringbuf *instance = (const struct ringbuf *)handle;

	if (get_length(instance) < data_size + offset) {
		return 0;
	}

	size_t index = GET_INDEX(instance->outdex + offset, instance->capacity);
	size_t contiguous = instance->capacity - index;
	size_t remained = (contiguous < data_size)? data_size - contiguous : 0;
	size_t cut = data_size - remained;

	memcpy(buf, &instance->buffer[index], cut);
	memcpy((uint8_t *)buf + cut, instance->buffer, remained);

	return data_size;
}

static bool consume_core(ringbuf_t *handle, size_t consume_size)
{
	struct ringbuf *instance = (struct ringbuf *)handle;

	if (get_length(instance) < consume_size) {
		return false;
	}

	instance->outdex += consume_size;

	return true;
}

size_t ringbuf_write(ringbuf_t *handle, const void *data, size_t data_size)
{
	struct ringbuf *instance = (struct ringbuf *)handle;

	if (get_available(instance) < data_size) {
		return 0;
	}

	size_t index = GET_INDEX(instance->index, instance->capacity);
	size_t contiguous = instance->capacity - index;
	size_t remained = (contiguous < data_size)? data_size - contiguous : 0;

	memcpy(instance->buffer + index, data, data_size - remained);
	memcpy(instance->buffer, ((const uint8_t *)data + data_size - remained),
			remained);

	instance->index += data_size;

	return data_size;
}

size_t ringbuf_write_cancel(ringbuf_t *handle, size_t size)
{
	struct ringbuf *instance = (struct ringbuf *)handle;

	if (get_length(instance) < size) {
		return 0;
	}

	instance->index -= size;

	return size;
}

size_t ringbuf_peek(const ringbuf_t *handle,
		size_t offset, void *buf, size_t data_size)
{
	return read_core(handle, offset, buf, data_size);
}

bool ringbuf_consume(ringbuf_t *handle, size_t consume_size)
{
	return consume_core(handle, consume_size);
}

size_t ringbuf_read(ringbuf_t *handle,
		size_t offset, void *buf, size_t data_size)
{
	size_t bytes_read = read_core(handle, offset, buf, data_size);

	if (bytes_read > 0) {
		consume_core(handle, bytes_read);
	}

	return bytes_read;
}

size_t ringbuf_length(const ringbuf_t *handle)
{
	return get_length((const struct ringbuf *)handle);
}

size_t ringbuf_capacity(const ringbuf_t *handle)
{
	return get_capacity((const struct ringbuf *)handle);
}

bool ringbuf_create_static(ringbuf_t *handle, void *buf, size_t bufsize)
{
	if (handle == NULL || buf == NULL || bufsize == 0) {
		return false;
	}

	struct ringbuf *p = (struct ringbuf *)handle;

	p->buffer = (uint8_t *)buf;
	initialize_instance(p, bufsize);

	return true;
}

#include <stdlib.h>

ringbuf_t *ringbuf_create(size_t space_size)
{
	struct ringbuf *instance;

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

	initialize_instance(instance, space_size);

	return (ringbuf_t *)instance;
}

void ringbuf_destroy(ringbuf_t *handle)
{
	struct ringbuf *instance = (struct ringbuf *)handle;
	free(instance->buffer);
	free(instance);
}
