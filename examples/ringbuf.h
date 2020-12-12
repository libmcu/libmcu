#ifndef RINGBUF_H
#define RINGBUF_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	size_t total_space;
	size_t space_used;
	size_t read_offset;
	uint8_t *space;
} ringbuf_t;

bool ringbuf_init(ringbuf_t *obj, void *space, size_t space_size);
size_t ringbuf_write(ringbuf_t *obj, const void *data, size_t data_size);
size_t ringbuf_write_cancel(ringbuf_t *obj, size_t size);
size_t ringbuf_read(ringbuf_t *obj, size_t offset, void *buf, size_t data_size);
bool ringbuf_consume(ringbuf_t *obj, size_t consume_size);
size_t ringbuf_used(ringbuf_t *obj);
size_t ringbuf_left(ringbuf_t *obj);

ringbuf_t *ringbuf_new(size_t space_size);
void ringbuf_delete(ringbuf_t *obj);

#if defined(__cplusplus)
}
#endif

#endif /* RINGBUF_H */
