/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
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

struct actor;
struct actor_msg;

typedef void (*actor_handler_t)(struct actor *self, struct actor_msg *msg);

struct actor {
	struct list link;
	actor_handler_t handler;
	struct actor_queue *queue;
	struct actor_msg *mailbox;
	int priority;
};

struct actor_queue {
	struct list messages;
};

int actor_init(void *msgpool, size_t msgpool_size, size_t stack_size_bytes);
int actor_deinit(void);

int actor_queue_init(struct actor_queue *queue);

/**
 * @brief Count how many messages in the queue
 *
 * @param queue queue
 *
 * @return number of messages
 */
int actor_queue_len(struct actor_queue *queue);

struct actor *actor_new(actor_handler_t handler, int priority,
		struct actor_queue *queue);
struct actor *actor_set(struct actor *actor, actor_handler_t handler,
		int priority, struct actor_queue *queue);

/**
 * @brief Send a message to an actor
 *
 * @param actor actor who gets the message
 * @param msg message to send
 *
 * @return 0 on success otherwise error
 */
int actor_send(struct actor *actor, struct actor_msg *msg);
int actor_send_defer(struct actor *actor, struct actor_msg *msg,
		uint32_t millisec_delay);

/**
 * @brief Allocate a memory block of requested size + header
 *
 * @param payload_size size of data to be used by application
 *
 * @return pointer of the memory block on success, NULL otherwise
 */
struct actor_msg *actor_alloc(size_t payload_size);
int actor_free(struct actor_msg *msg);
size_t actor_cap(void);
size_t actor_len(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ACTOR_H */
