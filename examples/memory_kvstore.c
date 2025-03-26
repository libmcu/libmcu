/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
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

struct kvstore {
	struct kvstore_api api;
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

static struct kvstore *find_namespace(char const *namespace)
{
	struct list *p;
	list_for_each(p, &namespace_list_head) {
		struct kvstore *obj =
			list_entry(p, typeof(*obj), namespace_list);
		if (!strncmp(obj->namespace, namespace,
					KVSTORE_MAX_NAMESPACE_LENGTH)) {
			return obj;
		}
	}
	return NULL;
}

static struct memory_kvstore_entry *find_key(struct kvstore const *obj,
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
	struct memory_kvstore_entry *entry;

	if ((entry = find_key(kvstore, key))) {
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
		list_add(&entry->list, &kvstore->keylist_head);
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
	struct memory_kvstore_entry const *entry = find_key(kvstore, key);
	if (entry) {
		size_t size = MIN(bufsize, entry->value_size);
		memcpy(buf, entry->value, size);
		return (int)size;
	}

	return 0;
}

static int memory_kvstore_open(struct kvstore *kvstore, const char *ns)
{
	(void)kvstore;
	return 0;
}

void memory_kvstore_destroy(struct kvstore *kvstore)
{
	struct list *p, *n;

	list_for_each_safe(p, n, &kvstore->keylist_head) {
		struct memory_kvstore_entry *entry =
			list_entry(p, typeof(*entry), list);
		free(entry->key);
		free(entry->value);
		free(entry);
	}

	list_del(&kvstore->namespace_list, &namespace_list_head);
	free(kvstore->namespace);
	free(kvstore);
}

struct kvstore *memory_kvstore_create(char const *ns)
{
	struct kvstore *p;

	if ((p = find_namespace(ns))) {
		return p;
	}

	size_t len = strnlen(ns, KVSTORE_MAX_NAMESPACE_LENGTH);

	if (!(p = malloc(sizeof(*p)))) {
		return NULL;
	}
	if (!(p->namespace = malloc(len+1))) {
		free(p);
		return NULL;
	}

	p->api.write = memory_kvstore_write;
	p->api.read = memory_kvstore_read;
	p->api.open = memory_kvstore_open;
	list_init(&p->keylist_head);
	strcpy(p->namespace, ns);
	p->namespace[len] = '\0';
	list_add(&p->namespace_list, &namespace_list_head);

	return p;
}

int memory_kvstore_init(void)
{
	return 0;
}
