#ifndef LIBMCU_NVS_KVSTORE_H
#define LIBMCU_NVS_KVSTORE_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/kvstore.h"

int nvs_kvstore_init(void);
kvstore_t *nvs_kvstore_new(const char *ns);
void nvs_kvstore_delete(kvstore_t *kvstore);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_NVS_KVSTORE_H */
