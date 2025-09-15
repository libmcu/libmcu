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

struct actor {
	struct list link;
	struct list messages;
	actor_handler_t handler;
	int priority;
};

/**
 * @brief Initialize the actor system.
 *
 * This function initializes the actor system with the provided memory
 * and stack size.
 *
 * @param[in] mem Pointer to the memory to be used by the actor system.
 * @param[in] memsize Size of the memory in bytes.
 * @param[in] stack_size_bytes Size of the stack for each actor in bytes.
 *
 * @return 0 on success, negative error code on failure.
 */
int actor_init(void *mem, const size_t memsize, const size_t stack_size_bytes);

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
 * @brief Allocate a memory block of requested size + header
 *
 * @param[in] payload_size size of data to be used by application
 *
 * @return pointer of the memory block on success, NULL otherwise
 */
struct actor_msg *actor_alloc(const size_t payload_size);
int actor_free(struct actor_msg *msg);

size_t actor_cap(void);
size_t actor_len(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ACTOR_H */
