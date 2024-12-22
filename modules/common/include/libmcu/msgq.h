/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_MSGQ_H
#define LIBMCU_MSGQ_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

typedef struct {
	size_t size;
} msgq_msg_meta_t;

typedef int (*msgq_lock_fn)(void *);
typedef int (*msgq_unlock_fn)(void *);

struct msgq;

/**
 * @brief Creates a message queue with the specified capacity.
 *
 * This function initializes a message queue with the given capacity in bytes.
 *
 * @note The capacity should be a power of 2 for optimal performance.
 *
 * @note The capacity of the message queue is fixed and cannot be changed after
 *       creation.
 *
 * @note A metadata overhead of 4 bytes is added to each message to store the
 *       size of the message.
 *
 * @param[in] capacity_bytes The maximum capacity of the queue in bytes.
 *
 * @return A pointer to the created message queue, or NULL if the creation
 *         fails.
 */
struct msgq *msgq_create(const size_t capacity_bytes);

/**
 * @brief Destroys the specified message queue.
 *
 * This function deinitializes the message queue and frees any allocated
 * resources.
 *
 * @param[in] q A pointer to the message queue to be destroyed.
 */
void msgq_destroy(struct msgq *q);

/**
 * @brief Sets synchronization functions for the message queue.
 *
 * This function sets custom lock and unlock functions for the message queue to
 * ensure thread safety. The provided context will be passed to the lock and
 * unlock functions.
 *
 * @param[in] q Pointer to the message queue.
 * @param[in] lock Function pointer to the lock function.
 * @param[in] unlock Function pointer to the unlock function.
 * @param[in] ctx Context to be passed to the lock and unlock functions.
 *
 * @return 0 on success, negative value on failure.
 */
int msgq_set_sync(struct msgq *q,
		msgq_lock_fn lock, msgq_unlock_fn unlock, void *ctx);

/**
 * @brief Pushes a message onto the message queue.
 *
 * This function adds a message to the message queue. The message queue uses a
 * hard-copy approach by default, meaning it copies the message data into the
 * queue. However, if the user passes a pointer to the data, the queue will
 * store the pointer, effectively using a soft-copy approach.
 *
 * @param[in] q Pointer to the message queue.
 * @param[in] data Pointer to the message data.
 * @param[in] datasize Size of the message data.
 *
 * @return 0 on success, negative value on failure.
 */
int msgq_push(struct msgq *q, const void *data, const size_t datasize);

/**
 * @brief Pops a message from the message queue.
 *
 * This function removes a message from the message queue and copies it into
 * the provided buffer.
 *
 * @note The function uses a hard-copy approach, meaning it copies the message
 *       data into the buffer. Ensure that the buffer is large enough to hold
 *       the message data.
 *
 * @param[in] q Pointer to the message queue.
 * @param[out] buf Buffer to copy the message data into.
 * @param[in] bufsize Size of the buffer.
 *
 * @return The length of the message read on success, negative value on failure.
 */
int msgq_pop(struct msgq *q, void *buf, size_t bufsize);

/**
 * @brief Returns the capacity of the message queue.
 *
 * This function returns the maximum number of bytes that the message queue can
 * hold. The capacity is determined during the initialization of the queue.
 *
 * @param[in] q Pointer to the message queue.
 *
 * @return The capacity of the message queue in bytes.
 */
size_t msgq_cap(const struct msgq *q);

/**
 * @brief Returns the current length of the message queue.
 *
 * This function returns the number of bytes currently stored in the message
 * queue.
 *
 * @param[in] q Pointer to the message queue.
 *
 * @return The current length of the message queue in bytes.
 */
size_t msgq_len(const struct msgq *q);

/**
 * @brief Get the number of available bytes in the queue.
 *
 * This function returns the number of bytes that can be read from
 * the message queue without blocking.
 *
 * @param[in] q Pointer to the message queue structure.
 *
 * @return The number of available bytes in the queue.
 */
size_t msgq_available(const struct msgq *q);

/**
 * @brief Returns the size of the next message in the message queue.
 *
 * This function returns the size of the next message that can be read from
 * the message queue without removing it. It allows the caller to determine
 * the size of the next message before actually reading it. It returns only the
 * size of the message, not including the metadata overhead.
 *
 * @param[in] q Pointer to the message queue.
 *
 * @return The size of the next message in bytes, or 0 if the queue is empty.
 */
size_t msgq_next_msg_size(const struct msgq *q);

/**
 * @brief Calculate the total size of the message queue.
 *
 * This function calculates the total size required for a message queue
 * based on the number of messages and the maximum size of each message.
 *
 * @param[in] n The number of messages in the queue.
 * @param[in] max_msg_size The maximum size of each message.
 *
 * @return The total size required for the message queue.
 */
size_t msgq_calc_size(const size_t n, const size_t max_msg_size);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_MSGQ_H */
