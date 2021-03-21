#ifndef LIBMCU_RINGBUF_H
#define LIBMCU_RINGBUF_H 202012L

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
} ringbuf_t;

bool ringbuf_init(ringbuf_t *self, void *buf, size_t bufsize);
size_t ringbuf_write(ringbuf_t *self, const void *data, size_t data_size);
size_t ringbuf_write_cancel(ringbuf_t *self, size_t size);
size_t ringbuf_read(const ringbuf_t *self,
		size_t offset, void *buf, size_t data_size);
bool ringbuf_consume(ringbuf_t *self, size_t consume_size);
size_t ringbuf_length(const ringbuf_t *self);
size_t ringbuf_capacity(const ringbuf_t *self);

ringbuf_t *ringbuf_new(size_t space_size);
void ringbuf_delete(ringbuf_t *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RINGBUF_H */
