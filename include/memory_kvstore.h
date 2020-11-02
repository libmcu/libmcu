#ifndef MEMORY_KVSTORE_H
#define MEMORY_KVSTORE_H 1

#include "kvstore.h"

int memory_kvstore_init(void);
void memory_kvstore_delete(kvstore_t *kvstore);
kvstore_t *memory_kvstore_new(const char *ns);

#endif /* MEMORY_KVSTORE_H */
