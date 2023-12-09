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

static struct actor_context {
	struct core core[ACTOR_PRIORITY_MAX];
	struct msgpool msgpool;
} m;

static int add_to_list(struct list *node, struct list *head)
{
	struct list *p;

	list_for_each(p, head) {
		if (p == node) {
			ACTOR_WARN("the entry(%p) exists in the queue(%p)",
					node, head);
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

static int schedule_actor(struct actor *actor, struct actor_context *ctx)
{
	struct core *core = &ctx->core[actor->priority];

	if (add_to_list(&actor->link, &core->runq) == 0) {
		sem_post(&core->dispatch_event);
	}

	return 0;
}

static void dispatch_actor(struct core *core)
{
	struct actor *actor = NULL;
	struct actor_msg *message = NULL;

	actor_lock();

	actor = pop_actor(&core->runq);
	message = (struct actor_msg *)(void *)
		pop_message(&actor->messages)->payload;

	actor_unlock();

	ACTOR_DEBUG("dispatch(%d) %p: %p", core->priority, actor, message);

	if (actor && actor->handler) {
		(*actor->handler)(actor, message);
	}
}

static void *dispatcher(void *e)
{
	struct core *core = (struct core *)e;

	ACTOR_INFO("Dispatcher(%d) started", core->priority);

	while (1) {
		sem_wait(&core->dispatch_event);
		dispatch_actor(core);
	}

	pthread_exit(NULL);
	return NULL;
}

static int initialize_scheduler(struct actor_context *ctx,
		size_t stack_size_bytes)
{
	for (int i = 0; i < ACTOR_PRIORITY_MAX; i++) {
		struct core *core = &ctx->core[i];

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

static int deinitialize_scheduler(struct actor_context *ctx)
{
	for (int i = 0; i < ACTOR_PRIORITY_MAX; i++) {
		struct core *core = &ctx->core[i];
#if defined(UNIT_TEST)
		pthread_cancel(core->thread);
#endif
		sem_destroy(&core->dispatch_event);
	}

	return 0;
}

struct actor_msg *actor_alloc(size_t payload_size)
{
	struct list *p = NULL;

	if (payload_size == 0 || payload_size > ACTOR_DEFAULT_MESSAGE_SIZE) {
		return NULL;
	}

	actor_lock();

	struct list *head = &m.msgpool.free_list.head;
	struct list *next = head->next;

	if (next != head) {
		p = next;
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

	struct list *head = &m.msgpool.free_list.head;
	struct sized_msg *p = list_entry(msg, struct sized_msg, payload);
	ACTOR_INFO("Free: %p (%p)", p, p->payload);

	actor_lock();

	add_to_list(&p->header.link, head);

	actor_unlock();

	return 0;
}

size_t actor_cap(void)
{
	return m.msgpool.cap;
}

size_t actor_len(void)
{
	struct list *head = &m.msgpool.free_list.head;

	actor_lock();

	size_t cnt = (size_t)list_count(head);

	actor_unlock();

	return m.msgpool.cap - cnt * sizeof(struct sized_msg);
}

int actor_send(struct actor *actor, struct actor_msg *msg)
{
	assert(actor);
	assert(msg);

	bool need_schedule = true;

	actor_lock();

	struct sized_msg *p = list_entry(msg, struct sized_msg, payload);
	if (add_to_list(&p->header.link, &actor->messages)) {
		need_schedule = false;
	}

	if (need_schedule) {
		schedule_actor(actor, &m);
	}

	actor_unlock();

	return 0;
}

int actor_send_defer(struct actor *actor, struct actor_msg *msg,
		uint32_t millisec_delay)
{
	unused(actor);
	unused(msg);
	unused(millisec_delay);
	return 0;
}

struct actor *actor_set(struct actor *actor,
		actor_handler_t handler, int priority)
{
	assert(actor);
	assert(handler);
	assert(priority < ACTOR_PRIORITY_MAX);

	actor->handler = handler;
	actor->priority = priority;

	list_init(&actor->link);
	list_init(&actor->messages);

	return actor;
}

int actor_init(void *mem, size_t memsize, size_t stack_size_bytes)
{
	const size_t mask = sizeof(uintptr_t) - 1;
	const size_t remainder = (size_t)mem & mask;

	assert(mem);
	assert(memsize >= (sizeof(struct actor_msg) + remainder));

	memset(&m, 0, sizeof(m));
	m.msgpool.buf = (void *)(((uintptr_t)mem + mask) & ~mask);
	m.msgpool.cap = memsize - remainder;

	struct list *free_list_head = &m.msgpool.free_list.head;
	struct sized_msg *sized_pool = (struct sized_msg *)m.msgpool.buf;
	size_t max_index = m.msgpool.cap / sizeof(*sized_pool);

	list_init(free_list_head);

	for (size_t i = 0; i < max_index; i++) {
		add_to_list(&sized_pool[i].header.link, free_list_head);
		ACTOR_DEBUG("free entry: %p", &sized_pool[i].header.link);
	}

	m.msgpool.cap = max_index * sizeof(*sized_pool);

	ACTOR_INFO("%lu free entries initialized.", max_index);
	ACTOR_DEBUG("%lu bytes wasted.", memsize - m.msgpool.cap);

	return initialize_scheduler(&m, stack_size_bytes);
}

int actor_deinit(void)
{
	return deinitialize_scheduler(&m);
}
