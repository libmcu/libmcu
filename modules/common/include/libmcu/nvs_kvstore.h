#ifndef NVS_KVSTORE_H
#define NVS_KVSTORE_H

#include "libmcu/kvstore.h"

struct kvstore *nvs_kvstore_new(void);
int nvs_kvstore_count(void);

#endif
