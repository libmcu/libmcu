#ifndef NVS_KVSTORE_H
#define NVS_KVSTORE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "kvstore.h"

int nvs_kvstore_init(void);
kvstore_t *nvs_kvstore_new(const char *ns);
void nvs_kvstore_delete(kvstore_t *kvstore);

#if defined(__cplusplus)
}
#endif

#endif /* NVS_KVSTORE_H */
