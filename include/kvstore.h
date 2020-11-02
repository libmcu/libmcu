#ifndef KVSTORE_H
#define KVSTORE_H 1

#include <stddef.h>

typedef struct kvstore {
	size_t (*write)(struct kvstore *kvstore,
			const char *key, const void *value, size_t size);
	size_t (*read)(const struct kvstore *kvstore,
			const char *key, void *buf, size_t bufsize);
} kvstore_t;

static inline size_t kvstore_write(kvstore_t *kvstore,
		const char *key, const void *value, size_t size)
{
	return kvstore->write(kvstore, key, value, size);
}

static inline size_t kvstore_read(const kvstore_t *kvstore,
		const char *key, void *buf, size_t bufsize)
{
	return kvstore->read(kvstore, key, buf, bufsize);
}

#endif /* KVSTORE_H */
