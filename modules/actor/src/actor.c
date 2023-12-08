/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/actor.h"
#include "libmcu/actor_overrides.h"

#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>

#include "libmcu/assert.h"
#include "libmcu/compiler.h"

#if !defined(ACTOR_DEBUG)
#define ACTOR_DEBUG(...)
#endif
#if !defined(ACTOR_INFO)
#define ACTOR_INFO(...)
#endif
#if !defined(ACTOR_WARN)
#define ACTOR_WARN(...)
#endif

#if !defined(ACTOR_PRIORITY_MAX)
#define ACTOR_PRIORITY_MAX		1
#endif
#if !defined(ACTOR_PRIORITY_BASE)
#define ACTOR_PRIORITY_BASE		0
#endif

struct actor_msg {
	struct list link;
};

#if !defined(ACTOR_DEFAULT_MESSAGE_SIZE)
#define ACTOR_DEFAULT_MESSAGE_SIZE		\
	(sizeof(struct actor_msg) + sizeof(uintptr_t))
#endif
LIBMCU_STATIC_ASSERT((ACTOR_DEFAULT_MESSAGE_SIZE % sizeof(uintptr_t)) == 0,
		"ACTOR_DEFAULT_MESSAGE_SIZE should be aligned to system memory alignment.");

struct sized_msg {
	struct actor_msg header;
	uint8_t payload[ACTOR_DEFAULT_MESSAGE_SIZE];
};

struct msg_free_list {
	struct list head;
};

struct msgpool {
	void *buf;
	size_t cap;
	size_t len;

	struct msg_free_list free_list;
};

struct core {
	struct list runq;

	pthread_t thread;
	pthread_attr_t thread_attr;
	sem_t dispatch_event;

	int priority;
};

static struct {
	struct core core[ACTOR_PRIORITY_MAX];
	struct msgpool msgpool;
} ctx;

static int add_to_list(struct list *node, struct list *head)
{
	struct list *p;

	list_for_each(p, head) {
		if (p == node) {
			ACTOR_WARN("node(%p) exists in the queue(%p)", node, head);
			return -EALREADY;
		}
	}

	list_add_tail(node, head);

	return 0;
}

static struct actor *pop_actor(struct list *q)
{
	if (list_empty(q)) {
		return NULL;
	}

	struct actor *actor = list_entry(list_first(q), struct actor, link);
	list_del(&actor->link, q);

	return actor;
}

static struct sized_msg *pop_message(struct list *q)
{
	if (list_empty(q)) {
		return NULL;
	}

	struct sized_msg *msg = list_entry(list_first(q),
			struct sized_msg, header);
	list_del(&msg->header.link, q);

	return msg;
}

static int schedule_actor(struct actor *actor)
{
	struct core *core = &ctx.core[actor->priority];

	actor->mailbox = (struct actor_msg *)(void *)
		pop_message(&actor->queue->messages)->payload;

	if (add_to_list(&actor->link, &core->runq) == 0) {
		sem_post(&core->dispatch_event);
	}

	return 0;
}

static void *dispatcher(void *e)
{
	struct core *core = (struct core *)e;

	ACTOR_INFO("Dispatcher(%d) started", core->priority);

	while (1) {
		struct actor *actor = NULL;

		sem_wait(&core->dispatch_event);

		actor_lock();
		actor = pop_actor(&core->runq);
		actor_unlock();

		ACTOR_DEBUG("dispatch(%d) %p", core->priority, actor);

		if (actor && actor->handler) {
			(*actor->handler)(actor, actor->mailbox);
		}
	}

	pthread_exit(NULL);
	return NULL;
}

static int initialize_scheduler(size_t stack_size_bytes)
{
	for (int i = 0; i < ACTOR_PRIORITY_MAX; i++) {
		struct core *core = &ctx.core[i];

		int rc = sem_init(&core->dispatch_event, 0, 0);
		assert(rc == 0);

		list_init(&core->runq);

#if defined(ACTOR_PRIORITY_DESCENDING)
		core->priority = ACTOR_PRIORITY_BASE + ACTOR_PRIORITY_MAX - i;
#else
		core->priority = ACTOR_PRIORITY_BASE + i;
#endif

		struct sched_param param;
		pthread_attr_init(&core->thread_attr);
		pthread_attr_getschedparam(&core->thread_attr, &param);
		param.sched_priority = core->priority;
		pthread_attr_setschedparam(&core->thread_attr, &param);
		pthread_attr_setstacksize(&core->thread_attr, stack_size_bytes);

		rc = pthread_create(&core->thread, &core->thread_attr,
				dispatcher, core);
		assert(rc == 0);
	}

	return 0;
}

static int deinitialize_scheduler(void)
{
	for (int i = 0; i < ACTOR_PRIORITY_MAX; i++) {
		struct core *core = &ctx.core[i];
#if defined(UNIT_TEST)
		pthread_cancel(core->thread);
#endif
		sem_destroy(&core->dispatch_event);
	}

	return 0;
}

struct actor_msg *actor_alloc(size_t payload_size)
{
	if (payload_size == 0 || payload_size > ACTOR_DEFAULT_MESSAGE_SIZE) {
		return NULL;
	}

	actor_lock();

	struct list *head = &ctx.msgpool.free_list.head;
	struct list *p = head->next;

	if (p && p != head) {
		list_del(p, head);
	}

	actor_unlock();

	if (p) {
		struct sized_msg *msg = (struct sized_msg *)p;
		ACTOR_INFO("Allocated: %p (%p)", msg, msg->payload);

		return (struct actor_msg *)(void *)msg->payload;
	}

	return NULL;
}

int actor_free(struct actor_msg *msg)
{
	assert(msg);

	struct list *head = &ctx.msgpool.free_list.head;
	struct sized_msg *p = list_entry(msg, struct sized_msg, payload);
	ACTOR_INFO("Free: %p (%p)", p, p->payload);

	actor_lock();

	add_to_list(&p->header.link, head);

	actor_unlock();

	return 0;
}

size_t actor_cap(void)
{
	return ctx.msgpool.cap;
}

size_t actor_len(void)
{
	struct list *head = &ctx.msgpool.free_list.head;

	actor_lock();

	size_t cnt = (size_t)list_count(head);

	actor_unlock();

	return ctx.msgpool.cap - cnt * sizeof(struct sized_msg);
}

int actor_queue_len(struct actor_queue *queue)
{
	actor_lock();
	int len = list_count(&queue->messages);
	actor_unlock();

	return len;
}

int actor_queue_init(struct actor_queue *queue)
{
	assert(queue);

	list_init(&queue->messages);

	return 0;
}

int actor_send(struct actor *actor, struct actor_msg *msg)
{
	assert(actor);
	assert(msg);

	bool need_schedule = true;

	actor_lock();

	struct sized_msg *p = list_entry(msg, struct sized_msg, payload);
	if (add_to_list(&p->header.link, &actor->queue->messages)) {
		need_schedule = false;
	}

	if (need_schedule) {
		schedule_actor(actor);
	}

	actor_unlock();

	return 0;
}

int actor_send_defer(struct actor *actor, struct actor_msg *msg,
		uint32_t millisec_delay)
{
	return 0;
}

struct actor *actor_set(struct actor *actor, actor_handler_t handler,
		int priority, struct actor_queue *queue)
{
	assert(actor);
	assert(handler);
	assert(priority < ACTOR_PRIORITY_MAX);
	assert(queue);

	actor->handler = handler;
	actor->mailbox = NULL;
	actor->queue = queue;
	actor->priority = priority;

	list_init(&actor->link);

	return actor;
}

int actor_init(void *msgpool, size_t msgpool_size, size_t stack_size_bytes)
{
	const size_t mask = sizeof(uintptr_t) - 1;
	const size_t remainder = (size_t)msgpool & mask;

	assert(msgpool);
	assert(msgpool_size >= (sizeof(struct actor_msg) - remainder));

	memset(&ctx, 0, sizeof(ctx));
	ctx.msgpool.buf = (void *)(((uintptr_t)msgpool + mask) & ~mask);
	ctx.msgpool.cap = msgpool_size - remainder;

	struct list *free_list_head = &ctx.msgpool.free_list.head;
	struct sized_msg *sized_pool = (struct sized_msg *)ctx.msgpool.buf;
	size_t max_index = ctx.msgpool.cap / sizeof(*sized_pool);

	list_init(free_list_head);

	for (size_t i = 0; i < max_index; i++) {
		add_to_list(&sized_pool[i].header.link, free_list_head);
		ACTOR_DEBUG("free list entry: %p", &sized_pool[i].header.link);
	}

	ctx.msgpool.cap = max_index * sizeof(*sized_pool);

	ACTOR_INFO("%lu free entries initialized.", max_index);
	ACTOR_DEBUG("%lu bytes will not be used.", msgpool_size - ctx.msgpool.cap);

	return initialize_scheduler(stack_size_bytes);
}

int actor_deinit(void)
{
	return deinitialize_scheduler();
}
