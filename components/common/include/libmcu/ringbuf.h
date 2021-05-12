#ifndef LIBMCU_RINGBUF_H
#define LIBMCU_RINGBUF_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	size_t capacity;
	size_t index;
	size_t outdex;
	uint8_t *buffer;
} ringbuf_static_t;

typedef ringbuf_static_t * ringbuf_t;

size_t ringbuf_write(ringbuf_t handle, const void *data, size_t data_size);
size_t ringbuf_write_cancel(ringbuf_t handle, size_t size);
size_t ringbuf_read(const ringbuf_t handle,
		size_t offset, void *buf, size_t data_size);
bool ringbuf_consume(ringbuf_t handle, size_t consume_size);
size_t ringbuf_length(const ringbuf_t handle);
size_t ringbuf_capacity(const ringbuf_t handle);

bool ringbuf_create_static(ringbuf_static_t *instance, void *buf, size_t bufsize);
ringbuf_t ringbuf_create(size_t space_size);
void ringbuf_destroy(ringbuf_t handle);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RINGBUF_H */
