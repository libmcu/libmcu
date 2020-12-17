#ifndef LIBMCU_MEMORY_KVSTORE_H
#define LIBMCU_MEMORY_KVSTORE_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/kvstore.h"

int memory_kvstore_init(void);
void memory_kvstore_delete(kvstore_t *kvstore);
kvstore_t *memory_kvstore_new(const char *ns);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_MEMORY_KVSTORE_H */
