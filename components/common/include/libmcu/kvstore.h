#ifndef LIBMCU_KVSTORE_H
#define LIBMCU_KVSTORE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

typedef struct kvstore_io * kvstore_t;

struct kvstore_io {
	size_t (*write)(kvstore_t kvstore,
			const char *key, const void *value, size_t size);
	size_t (*read)(const kvstore_t kvstore,
			const char *key, void *buf, size_t bufsize);
};

static inline size_t kvstore_write(kvstore_t kvstore,
		const char *key, const void *value, size_t size)
{
	return kvstore->write(kvstore, key, value, size);
}

static inline size_t kvstore_read(const kvstore_t kvstore,
		const char *key, void *buf, size_t bufsize)
{
	return kvstore->read(kvstore, key, buf, bufsize);
}

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_KVSTORE_H */
