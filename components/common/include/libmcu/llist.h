#ifndef LIBMCU_DOUBLY_LINKED_LIST_H
#define LIBMCU_DOUBLY_LINKED_LIST_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "libmcu/compiler.h"

#define LLIST_POISON_NEXT		((struct llist *)0xfeedbac0)
#define LLIST_POISON_PREV		((struct llist *)0xfeedbac1)

#define llist_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)
#define llist_for_each_safe(pos, tmp, head) \
	for (pos = (head)->next, tmp = pos->next; pos != (head); \
			pos = tmp, tmp = pos->next)
#define llist_entry(ptr, type, member) \
	((type *)(void *)((char *)(ptr) - offsetof(type, member)))

#define DEFINE_LLIST_HEAD(name)		struct llist name = { \
	.next = &name, \
	.prev = &name, \
}

struct llist {
	struct llist *next;
	struct llist *prev;
};

static inline LIBMCU_ALWAYS_INLINE void llist_init(struct llist *head)
{
	head->next = head;
	head->prev = head;
}

static inline LIBMCU_ALWAYS_INLINE void llist_add(struct llist *node,
		struct llist *ref)
{
	if (!node || !ref) {
		return;
	}

	node->next = ref->next;
	node->prev = ref;
	ref->next->prev = node;
	ref->next = node;
}

static inline LIBMCU_ALWAYS_INLINE void llist_add_tail(struct llist *node,
		struct llist *ref)
{
	if (!node || !ref) {
		return;
	}

	node->next = ref->prev->next;
	node->prev = ref->prev;
	ref->prev->next = node;
	ref->prev = node;
}

static inline LIBMCU_ALWAYS_INLINE void llist_del(struct llist *node)
{
	if (!node) {
		return;
	}

	node->next->prev = node->prev;
	node->prev->next = node->next;

	node->next = LLIST_POISON_NEXT;
	node->prev = LLIST_POISON_PREV;
}

static inline LIBMCU_ALWAYS_INLINE bool llist_empty(const struct llist *ref)
{
	if (ref && ref->next == ref && ref->prev == ref) {
		return true;
	}
	return false;
}

static inline LIBMCU_ALWAYS_INLINE int llist_count(struct llist *ref)
{
	int n = 0;
	struct llist *p;
	llist_for_each(p, ref) {
		n++;
	}

	return n;
}

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_DOUBLY_LINKED_LIST_H */
