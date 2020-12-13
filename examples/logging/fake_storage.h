#ifndef LOGGING_FAKE_STORAGE_H
#define LOGGING_FAKE_STORAGE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/logging_storage.h"

const logging_storage_t *logging_fake_storage_init(
		int (*_print_string)(const char *s));

#if defined(__cplusplus)
}
#endif

#endif /* LOGGING_FAKE_STORAGE_H */
