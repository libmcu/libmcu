/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
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
	static struct ringbuf _name = { \
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
 * This function writes the specified amount of data to the ring buffer.
 * If the buffer does not have enough space to accommodate the data,
 * it will write as much as possible.
 *
 * @param[in] handle Pointer to the ring buffer handle.
 * @param[in] data Pointer to the data to be written to the ring buffer.
 * @param[in] datasize Size of the data to be written, in bytes.
 *
 * @return The number of bytes actually written to the ring buffer.
 */
size_t ringbuf_write(struct ringbuf *handle,
		const void *data, const size_t datasize);

/**
 * @brief Cancels the write operation on the ring buffer.
 *
 * This function cancels the specified amount of data that was previously
 * written to the ring buffer. It effectively reduces the write pointer
 * by the specified size, making the space available for future writes.
 *
 * @param[in] handle Pointer to the ring buffer handle.
 * @param[in] size The size of the data to cancel, in bytes.
 *
 * @return The number of bytes actually canceled from the ring buffer.
 */
size_t ringbuf_write_cancel(struct ringbuf *handle, const size_t size);

/**
 * @brief Peeks at the data in the ring buffer without removing it.
 *
 * This function allows you to read data from the ring buffer starting
 * from a specified offset without advancing the read pointer. It copies
 * the data into the provided buffer up to the specified buffer size.
 *
 * @param[in] handle Pointer to the ring buffer handle.
 * @param[in] offset The offset from the current read pointer to start peeking.
 * @param[out] buf Pointer to the buffer where the peeked data will be copied.
 * @param[in] bufsize The size of the buffer, in bytes.
 *
 * @return The number of bytes actually copied to the buffer.
 */
size_t ringbuf_peek(const struct ringbuf *handle,
		const size_t offset, void *buf, const size_t bufsize);

/**
 * @brief Peeks at the data in the ring buffer and returns a pointer.
 *
 * This function returns a pointer to the data in the ring buffer starting
 * from a specified offset. This allows direct access to the data without
 * copying it to another buffer. The caller must ensure that the data is
 * not modified while it is being accessed.
 *
 * @param[in] handle Pointer to the ring buffer handle.
 * @param[in] offset The offset from the current read pointer to start
 *            accessing.
 * @param[out] contiguous Pointer to a variable where the size of
 *             the accessible contiguous data will be stored.
 *
 * @return Pointer to the data in the ring buffer, or NULL if the offset is
 *         invalid.
 */
const void *ringbuf_peek_pointer(const struct ringbuf *handle,
		const size_t offset, size_t *contiguous);

/**
 * @brief Consumes data from the ring buffer.
 *
 * This function advances the read pointer by the specified amount,
 * effectively consuming the data from the ring buffer. The consumed
 * data is no longer available for reading.
 *
 * @param[in] handle Pointer to the ring buffer handle.
 * @param[in] consume_size The size of the data to consume, in bytes.
 *
 * @return true if the operation was successful, false if the consume size
 *         exceeds the available data in the buffer.
 */
bool ringbuf_consume(struct ringbuf *handle, const size_t consume_size);

/**
 * @brief Reads data from the ring buffer.
 *
 * This function reads data from the ring buffer starting from a specified
 * offset and copies it into the provided buffer. The read pointer is not
 * advanced by this operation.
 *
 * @param[in] handle Pointer to the ring buffer handle.
 * @param[in] offset The offset from the current read pointer to start reading.
 * @param[out] buf Pointer to the buffer where the read data will be copied.
 * @param[in] bufsize The size of the buffer, in bytes.
 *
 * @return The number of bytes actually read from the ring buffer.
 */
size_t ringbuf_read(struct ringbuf *handle,
		const size_t offset, void *buf, const size_t bufsize);

/**
 * @brief Gets the current length of data in the ring buffer.
 *
 * This function returns the number of bytes of data currently stored
 * in the ring buffer, which is the amount of data available for reading.
 *
 * @param[in] handle Pointer to the ring buffer handle.
 *
 * @return The number of bytes of data currently in the ring buffer.
 */
size_t ringbuf_length(const struct ringbuf *handle);

/**
 * @brief Gets the total capacity of the ring buffer.
 *
 * This function returns the total capacity of the ring buffer, which is
 * the maximum amount of data that can be stored in the buffer at any time.
 *
 * @param[in] handle Pointer to the ring buffer handle.
 *
 * @return The total capacity of the ring buffer, in bytes.
 */
size_t ringbuf_capacity(const struct ringbuf *handle);

/**
 * @brief Initializes a ring buffer with a static buffer.
 *
 * This function initializes a ring buffer using a statically allocated
 * buffer provided by the caller. The buffer size must be specified, and
 * the ring buffer handle will be initialized to manage this buffer.
 *
 * @param[in] handle Pointer to the ring buffer handle to be initialized.
 * @param[out] buf Pointer to the statically allocated buffer.
 * @param[in] bufsize The size of the static buffer, in bytes.
 *
 * @note The size of the buffer must be a power of 2. If the specified size is
 *       not a power of 2, the function will return false to indicate that a
 *       larger buffer (rounded up to the next power of 2) is required.
 *
 * @note The buffer must be statically allocated and must remain valid for the
 *       lifetime of the ring buffer.
 *
 * @note The buffer should be aligned to the maximum alignment requirement of
 *       the system. For example, if the system requires 4-byte alignment,
 *       the buffer should be aligned to a 4-byte boundary.
 *
 * @return true if the ring buffer was successfully initialized, false
 *         otherwise (including when bufsize is not a power of 2).
 */
bool ringbuf_create_static(struct ringbuf *handle,
		void *buf, const size_t bufsize);

/**
 * @brief Creates a ring buffer with a dynamically allocated buffer.
 *
 * This function creates a ring buffer by dynamically allocating a buffer
 * of the specified size. The ring buffer handle will be initialized to
 * manage this buffer.
 *
 * @param[in] space_size The size of the buffer to be allocated, in bytes.
 *
 * @note If the specified size is not a power of 2, the actual buffer size
 *       will be automatically rounded up to the next power of 2 to ensure
 *       the full requested capacity is available. For example, requesting
 *       100 bytes will allocate 128 bytes.
 *
 * @return Pointer to the newly created ring buffer handle, or NULL if
 *         the allocation fails.
 */
struct ringbuf *ringbuf_create(const size_t space_size);

/**
 * @brief Destroys a ring buffer and frees its resources.
 *
 * This function destroys the specified ring buffer and frees any
 * dynamically allocated memory associated with it. The ring buffer
 * handle should not be used after this function is called.
 *
 * @param handle Pointer to the ring buffer handle to be destroyed.
 */
void ringbuf_destroy(struct ringbuf *handle);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RINGBUF_H */
