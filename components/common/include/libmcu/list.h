#ifndef LIBMCU_LINKED_LIST_H
#define LIBMCU_LINKED_LIST_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "libmcu/compiler.h"

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)
#define list_for_each_safe(pos, tmp, head) \
	for (pos = (head)->next, tmp = pos->next; pos != (head); \
			pos = tmp, tmp = pos->next)
#define list_entry(ptr, type, member) \
	((type *)(void *)((char *)(ptr) - offsetof(type, member)))

#define DEFINE_LIST_HEAD(name)		struct list name = { .next = &name, }

struct list {
	struct list *next;
};

static inline LIBMCU_ALWAYS_INLINE void list_init(struct list *head)
{
	head->next = head;
}

static inline LIBMCU_ALWAYS_INLINE void list_add(struct list *node,
		struct list *head)
{
	node->next = head->next;
	head->next = node;
}

static inline LIBMCU_ALWAYS_INLINE void list_add_tail(struct list *node,
		struct list *head)
{
	struct list **ref = &head;

	while ((*ref)->next != head) {
		ref = &(*ref)->next;
	}

	node->next = (*ref)->next;
	(*ref)->next = node;
}

static inline LIBMCU_ALWAYS_INLINE int list_del(const struct list *node,
		struct list *head)
{
	struct list **ref = &head;

	while ((*ref)->next != head && (*ref)->next != node) {
		ref = &(*ref)->next;
	}

	if ((*ref)->next == head) {
		return -1;
	}

	(*ref)->next = (*ref)->next->next;
	return 0;
}

static inline LIBMCU_ALWAYS_INLINE bool list_empty(const struct list *head)
{
	if (head->next == head) {
		return true;
	}
	return false;
}

static inline LIBMCU_ALWAYS_INLINE int list_count(struct list *head)
{
	struct list *p;
	int n = 0;
	list_for_each(p, head) {
		n++;
	}
	return n;
}

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_LINKED_LIST_H */
