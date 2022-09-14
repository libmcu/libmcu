/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "memory_kvstore.h"

#include <stdlib.h>
#include <string.h>

#include "libmcu/list.h"

#undef MIN
#define MIN(x, y)				((x) > (y)? (y) : (x))

#if !defined(KVSTORE_MAX_KEY_LENGTH)
#define KVSTORE_MAX_KEY_LENGTH			32
#endif
#if !defined(KVSTORE_MAX_NAMESPACE_LENGTH)
#define KVSTORE_MAX_NAMESPACE_LENGTH		32
#endif

struct memory_kvstore {
	struct kvstore ops;
	char *namespace;
	struct list namespace_list;
	struct list keylist_head;
};

struct memory_kvstore_entry {
	struct list list;
	char *key;
	void *value;
	size_t value_size;
};

static DEFINE_LIST_HEAD(namespace_list_head);

static struct memory_kvstore *find_namespace(char const *namespace)
{
	struct list *p;
	list_for_each(p, &namespace_list_head) {
		struct memory_kvstore *obj =
			list_entry(p, typeof(*obj), namespace_list);
		if (!strncmp(obj->namespace, namespace,
					KVSTORE_MAX_NAMESPACE_LENGTH)) {
			return obj;
		}
	}
	return NULL;
}

static struct memory_kvstore_entry *find_key(struct memory_kvstore const *obj,
		char const *key)
{
	struct list *p;
	list_for_each(p, &obj->keylist_head) {
		struct memory_kvstore_entry *entry =
			list_entry(p, typeof(*entry), list);
		if (!strncmp(entry->key, key, KVSTORE_MAX_KEY_LENGTH)) {
			return entry;
		}
	}
	return NULL;
}

static int memory_kvstore_write(struct kvstore *kvstore,
		char const *key, void const *value, size_t size)
{
	struct memory_kvstore *obj = (struct memory_kvstore *)kvstore;

	struct memory_kvstore_entry *entry;
	if ((entry = find_key(obj, key))) {
		void *new_value = malloc(size);
		if (!new_value) {
			goto err;
		}
		free(entry->value);
		entry->value = new_value;
	} else {
		size_t len = strnlen(key, KVSTORE_MAX_KEY_LENGTH);

		if (!(entry = malloc(sizeof(*entry)))) {
			goto err;
		}
		if (!(entry->key = malloc(len+1))) {
			goto err_free_entry;
		}
		if (!(entry->value = malloc(size))) {
			goto err_free_key;
		}

		strcpy(entry->key, key);
		entry->key[len] = '\0';
		list_add(&entry->list, &obj->keylist_head);
	}

	memcpy(entry->value, value, size);
	entry->value_size = size;

	return (int)size;

err_free_key:
	free(entry->key);
err_free_entry:
	free(entry);
err:
	return 0;
}

static int memory_kvstore_read(struct kvstore *kvstore,
		char const *key, void *buf, size_t bufsize)
{
	struct memory_kvstore const *obj =
		(struct memory_kvstore const *)kvstore;
	struct memory_kvstore_entry const *entry = find_key(obj, key);
	if (entry) {
		size_t size = MIN(bufsize, entry->value_size);
		memcpy(buf, entry->value, size);
		return (int)size;
	}

	return 0;
}

void memory_kvstore_destroy(struct kvstore *kvstore)
{
	struct memory_kvstore *obj = (struct memory_kvstore *)kvstore;

	struct list *p, *n;
	list_for_each_safe(p, n, &obj->keylist_head) {
		struct memory_kvstore_entry *entry =
			list_entry(p, typeof(*entry), list);
		free(entry->key);
		free(entry->value);
		free(entry);
	}

	list_del(&obj->namespace_list, &namespace_list_head);
	free(obj->namespace);
	free(kvstore);
}

struct kvstore *memory_kvstore_create(char const *ns)
{
	struct memory_kvstore *p;

	if ((p = find_namespace(ns))) {
		return &p->ops;
	}

	size_t len = strnlen(ns, KVSTORE_MAX_NAMESPACE_LENGTH);

	if (!(p = malloc(sizeof(*p)))) {
		return NULL;
	}
	if (!(p->namespace = malloc(len+1))) {
		free(p);
		return NULL;
	}

	p->ops.write = memory_kvstore_write;
	p->ops.read = memory_kvstore_read;
	list_init(&p->keylist_head);
	strcpy(p->namespace, ns);
	p->namespace[len] = '\0';
	list_add(&p->namespace_list, &namespace_list_head);

	return &p->ops;
}

int memory_kvstore_init(void)
{
	return 0;
}
