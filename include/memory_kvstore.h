#ifndef MEMORY_KVSTORE_H
#define MEMORY_KVSTORE_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include "kvstore.h"

int memory_kvstore_init(void);
void memory_kvstore_delete(kvstore_t *kvstore);
kvstore_t *memory_kvstore_new(const char *ns);

#if defined(__cplusplus)
}
#endif

#endif /* MEMORY_KVSTORE_H */
