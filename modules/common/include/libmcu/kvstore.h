#ifndef LIBMCU_KVSTORE_H
#define LIBMCU_KVSTORE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

struct kvstore {
	int (*write)(struct kvstore *self,
			char const *key, void const *value, size_t size);
	int (*read)(struct kvstore const *self,
			char const *key, void *buf, size_t size);
	int (*open)(struct kvstore const *self, char const *ns);
	void (*close)(struct kvstore const *self);
};

static inline int kvstore_open(struct kvstore const *self, char const *ns)
{
	return self->open(self, ns);
}

static inline void kvstore_close(struct kvstore const *self)
{
	self->close(self);
}

static inline int kvstore_write(struct kvstore *self,
		char const *key, void const *value, size_t size)
{
	return self->write(self, key, value, size);
}

static inline int kvstore_read(struct kvstore const *self,
		char const *key, void *buf, size_t size)
{
	return self->read(self, key, buf, size);
}

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_KVSTORE_H */
