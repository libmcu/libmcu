/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_RINGBUF_H
#define LIBMCU_RINGBUF_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "libmcu/compiler.h"

struct ringbuf {
	size_t capacity;
	size_t index;
	size_t outdex;
	uint8_t *buffer;
};

#define DEFINE_RINGBUF(_name, _bufsize) \
	static uint8_t LIBMCU_CONCAT(_name, _buf)[_bufsize]; \
	struct ringbuf _name = { \
		.capacity = _bufsize, \
		.index = 0, \
		.outdex = 0, \
		.buffer = LIBMCU_CONCAT(_name, _buf), \
	}; \
	static_assert((_bufsize & (_bufsize - 1)) == 0, \
			      "_bufsize should be power of 2.")

/**
 * @brief Writes data to the ring buffer.
 *
 * @param[in] handle Pointer to the ring buffer instance.
 * @param[in] data Pointer to the data to write.
 * @param[in] datasize Size of the data to write.
 *
 * @return The number of bytes written to the ring buffer, zero for failure.
 */
size_t ringbuf_write(struct ringbuf *handle, const void *data, size_t datasize);

/**
 * @brief Cancels the last write operation on the ring buffer.
 *
 * @param[in] handle Pointer to the ring buffer instance.
 * @param[in] size Size of the data to cancel.
 *
 * @return The number of bytes cancelled.
 */
size_t ringbuf_write_cancel(struct ringbuf *handle, size_t size);

/**
 * @brief Peeks at data in the ring buffer without consuming it.
 *
 * @param[in] handle Pointer to the ring buffer instance.
 * @param[in] offset Offset from the start of the buffer to peek from.
 * @param[out] buf Buffer to store the peeked data.
 * @param[in] bufsize Size of the buffer.
 *
 * @return The number of bytes peeked.
 */
size_t ringbuf_peek(const struct ringbuf *handle,
		size_t offset, void *buf, size_t bufsize);

/**
 * @brief Consumes data from the ring buffer.
 *
 * @param[in] handle Pointer to the ring buffer instance.
 * @param[in] consume_size Size of the data to consume.
 *
 * @return True if the data was successfully consumed, false otherwise.
 */
bool ringbuf_consume(struct ringbuf *handle, size_t consume_size);

/**
 * @brief Reads data from the ring buffer.
 *
 * @param[in] handle Pointer to the ring buffer instance.
 * @param[in] offset Offset from the start of the buffer to read from.
 * @param[out] buf Buffer to store the read data.
 * @param[in] bufsize Size of the buffer.
 *
 * @return The number of bytes read.
 */
size_t ringbuf_read(struct ringbuf *handle,
		size_t offset, void *buf, size_t bufsize);

/**
 * @brief Gets the current length of data in the ring buffer.
 *
 * @param[in] handle Pointer to the ring buffer instance.
 *
 * @return The current length of data in the ring buffer.
 */
size_t ringbuf_length(const struct ringbuf *handle);

/**
 * @brief Gets the total capacity of the ring buffer.
 *
 * @param[in] handle Pointer to the ring buffer instance.
 *
 * @return The total capacity of the ring buffer.
 */
size_t ringbuf_capacity(const struct ringbuf *handle);

/**
 * @brief Creates a static ring buffer.
 *
 * @param[in] handle Pointer to the ring buffer instance.
 * @param[in] buf Buffer to use for the ring buffer.
 * @param[in] bufsize Size of the buffer.
 *
 * @return True if the ring buffer was successfully created, false otherwise.
 */
bool ringbuf_create_static(struct ringbuf *handle, void *buf, size_t bufsize);

/**
 * @brief Creates a dynamic ring buffer.
 *
 * @param[in] space_size Size of the space to allocate for the ring buffer.
 *
 * @return Pointer to the newly created ring buffer instance.
 */
struct ringbuf *ringbuf_create(size_t space_size);

/**
 * @brief Destroys a ring buffer.
 *
 * @param[in] handle Pointer to the ring buffer instance to destroy.
 */
void ringbuf_destroy(struct ringbuf *handle);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RINGBUF_H */
