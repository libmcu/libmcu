/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/cleanup.h"
#include <stdlib.h>
#include "libmcu/list.h"

struct cleanup {
	struct list link;

	cleanup_func_t func;
	void *func_ctx;

	int priority;
};

struct cleanup_manager {
	struct list list;
};

static struct cleanup_manager m;

void cleanup_execute(void)
{
	struct list *p;

	list_for_each(p, &m.list) {
		struct cleanup *entry = list_entry(p, struct cleanup, link);
		entry->func(entry->func_ctx);
	}
}

cleanup_error_t cleanup_register(int priority,
		cleanup_func_t func, void *func_ctx)
{
	struct cleanup *entry =
		(struct cleanup *)malloc(sizeof(struct cleanup));

	if (!entry) {
		return CLEANUP_ERROR_ALLOC_FAILED;
	}

	*entry = (struct cleanup) {
		.func = func,
		.func_ctx = func_ctx,
		.priority = priority,
	};

	struct list **p = &m.list.next;
	while (*p != &m.list) {
		struct cleanup *tmp = list_entry(*p, struct cleanup, link);
		if (priority > tmp->priority) {
			break;
		}
		p = &(*p)->next;
	}
	entry->link.next = *p;
	*p = &entry->link;

	return CLEANUP_ERROR_NONE;
}

cleanup_error_t cleanup_init(void)
{
	list_init(&m.list);
	return CLEANUP_ERROR_NONE;
}

void cleanup_deinit(void)
{
	struct list *p;
	struct list *t;

	list_for_each_safe(p, t, &m.list) {
		struct cleanup *entry = list_entry(p, struct cleanup, link);
		list_del(p, &m.list);
		free(entry);
	}
}
