#ifndef LOGGING_FAKE_STORAGE_H
#define LOGGING_FAKE_STORAGE_H

#include "libmcu/logging_storage.h"

const logging_storage_t *logging_fake_storage_init(
		void (*_print_string)(const char *s));

#endif /* LOGGING_FAKE_STORAGE_H */
