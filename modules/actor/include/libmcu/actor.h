/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_ACTOR_H
#define LIBMCU_ACTOR_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <pthread.h>
#include "libmcu/list.h"

#if !defined(ACTOR_PRIORITY_MAX)
#define ACTOR_PRIORITY_MAX		1
#endif
#if !defined(ACTOR_PRIORITY_BASE)
#define ACTOR_PRIORITY_BASE		1
#endif

#define ACTOR_DEFINE(name, fn, pri)				\
	struct actor name = {					\
		.handler = fn,					\
		.priority = pri,				\
		.link = { .next = &name.link, },		\
		.messages = { .next = &name.messages, },	\
	}

struct actor;
struct actor_msg;

typedef void (*actor_handler_t)(struct actor *self, struct actor_msg *msg);
typedef size_t (*actor_stack_size_getter_t)(int pri, void *ctx);

struct actor {
	struct list link;
	struct list messages;
	actor_handler_t handler;
	int priority;
	pthread_mutex_t mutex;
};

/**
 * @brief Initialize the actor system.
 *
 * This function initializes the actor system with the provided memory
 * and stack size. It allows dynamic determination of stack size for
 * each actor through a user-provided callback.
 *
 * @param[in] mem Pointer to the memory to be used by the actor system.
 *                This memory is used for message creation via actor_alloc
 *                and is managed internally by the actor system.
 * @param[in] memsize Size of the memory in bytes. It must be sufficient
 *                to accommodate the actor system's requirements.
 * @param[in] stack_size_getter Function pointer to a user-defined callback
 *                that determines the stack size for each priority level.
 *                This allows flexibility in stack allocation.
 * @param[in] stack_size_getter_ctx Context pointer passed to the stack size
 *                getter callback. This can be used to provide additional
 *                information or state to the callback.
 *
 * @return 0 on success, negative error code on failure.
 *         Possible error codes include:
 *         - -EINVAL: Invalid arguments provided.
 *         - -ENOMEM: Insufficient memory for initialization.
 */
int actor_init(void *mem, const size_t memsize,
		actor_stack_size_getter_t stack_size_getter,
		void *stack_size_getter_ctx);

/**
 * @brief Deinitialize the actor system.
 *
 * This function deinitializes the actor system, releasing any resources
 * that were allocated.
 *
 * @return 0 on success, negative error code on failure.
 */
int actor_deinit(void);

/**
 * @brief Create a new actor.
 *
 * This function creates a new actor with the specified handler and priority.
 *
 * @param[in] handler Function pointer to the actor's handler function.
 * @param[in] priority Priority of the actor.
 *
 * @return Pointer to the created actor instance, or NULL on failure.
 */
struct actor *actor_new(actor_handler_t handler, const int priority);

/**
 * @brief Deletes an actor instance and releases its resources.
 *
 * This function is used to delete an actor instance, freeing any
 * resources allocated to it. After calling this function, the actor
 * instance should no longer be used.
 *
 * @param[in] actor Pointer to the actor instance to be deleted.
 */
void actor_delete(struct actor *actor);

/**
 * @brief Set the handler and priority of an existing actor.
 *
 * This function sets the handler and priority of the specified actor.
 *
 * @param[in] actor Pointer to the actor instance.
 * @param[in] handler Function pointer to the actor's handler function.
 * @param[in] priority Priority of the actor.
 *
 * @return Pointer to the updated actor instance.
 */
struct actor *actor_set(struct actor *actor,
		actor_handler_t handler, const int priority);

/**
 * @brief Unsets or deactivates the specified actor instance.
 *
 * This function deactivates the given actor instance, effectively
 * removing it from the actor system. It should be used when the actor
 * is no longer needed or requires reinitialization.
 *
 * @param[in] actor Pointer to the actor instance to be unset.
 */
void actor_unset(struct actor *actor);

/**
 * @brief Send a message to an actor.
 *
 * This function sends a message to the specified actor.
 *
 * @param[in] actor Pointer to the actor instance.
 * @param[in] msg Pointer to the message to be sent.
 *
 * @return 0 on success, negative error code on failure.
 */
int actor_send(struct actor *actor, struct actor_msg *msg);

/**
 * @brief Send a deferred message to an actor.
 *
 * This function sends a message to the specified actor after a delay.
 *
 * @param[in] actor Pointer to the actor instance.
 * @param[in] msg Pointer to the message to be sent.
 * @param[in] millisec_delay Delay in milliseconds before the message is sent.
 *
 * @return 0 on success, negative error code on failure.
 */
int actor_send_defer(struct actor *actor,
		struct actor_msg *msg, const uint32_t millisec_delay);

/**
 * @brief Retrieves the number of messages managed by the given actor instance.
 *
 * This function returns the count of messages associated with the specified
 * actor instance. It can be used to monitor or debug the actor system.
 *
 * @param[in] actor Pointer to the actor instance whose message count is to be
 *                  retrieved.
 * @return The number of messages managed by the given actor instance.
 */
size_t actor_count_messages(struct actor *actor);

/**
 * @brief Allocate a memory block of requested size + header.
 *
 * @param[in] payload_size size of data to be used by application
 *
 * @return pointer of the memory block on success, NULL otherwise
 */
struct actor_msg *actor_alloc(const size_t payload_size);

/**
 * @brief Frees a message allocated for the actor.
 *
 * This function releases the memory and resources associated with the given
 * actor message. It should be called when the message is no longer needed
 * to avoid memory leaks.
 *
 * @param[in] msg Pointer to the actor message to be freed.
 * @return 0 on success, or a negative error code on failure.
 */
int actor_free(struct actor_msg *msg);

/**
 * @brief Retrieves the memory capacity allocated for the actor system.
 *
 * This function returns the total memory capacity (in bytes) that has been
 * allocated for the actor system. It can be used to monitor memory usage.
 *
 * @return The total memory capacity allocated for the actor system, in bytes.
 */
size_t actor_mem_cap(void);

/**
 * @brief Retrieves the current memory usage of the actor system.
 *
 * This function returns the amount of memory (in bytes) currently being used
 * by the actor system. It can be used to track memory consumption over time.
 *
 * @return The current memory usage of the actor system, in bytes.
 */
size_t actor_mem_len(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ACTOR_H */
