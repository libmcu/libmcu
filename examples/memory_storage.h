#ifndef LOGGING_MEMORY_STORAGE_H
#define LOGGING_MEMORY_STORAGE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/logging_storage.h"
#include <stddef.h>

const logging_storage_t *memory_storage_init(void *storage, size_t storage_size);
void memory_storage_deinit(void);

#if defined(__cplusplus)
}
#endif

#endif /* LOGGING_MEMORY_STORAGE_H */
