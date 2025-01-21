/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_KVSTORE_H
#define LIBMCU_KVSTORE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

struct kvstore;

struct kvstore_api {
	int (*write)(struct kvstore *self,
			char const *key, void const *value, size_t size);
	int (*read)(struct kvstore *self,
			char const *key, void *buf, size_t size);
	int (*clear)(struct kvstore *self, char const *key);
	int (*open)(struct kvstore *self, char const *ns);
	void (*close)(struct kvstore *self);
};

static inline int kvstore_open(struct kvstore *self, char const *ns) {
	return ((struct kvstore_api *)self)->open(self, ns);
}

static inline void kvstore_close(struct kvstore *self) {
	((struct kvstore_api *)self)->close(self);
}

static inline int kvstore_write(struct kvstore *self,
		char const *key, void const *value, size_t size) {
	return ((struct kvstore_api *)self)->write(self, key, value, size);
}

static inline int kvstore_read(struct kvstore *self,
		char const *key, void *buf, size_t size) {
	return ((struct kvstore_api *)self)->read(self, key, buf, size);
}

static inline int kvstore_clear(struct kvstore *self, char const *key) {
	return ((struct kvstore_api *)self)->clear(self, key);
}

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_KVSTORE_H */
