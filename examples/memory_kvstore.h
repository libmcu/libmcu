#ifndef LIBMCU_MEMORY_KVSTORE_H
#define LIBMCU_MEMORY_KVSTORE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/kvstore.h"

int memory_kvstore_init(void);
void memory_kvstore_destroy(struct kvstore *kvstore);
struct kvstore *memory_kvstore_create(const char *ns);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_MEMORY_KVSTORE_H */
