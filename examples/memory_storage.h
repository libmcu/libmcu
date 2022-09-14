/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef MEMORY_STORAGE_H
#define MEMORY_STORAGE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/logging_storage.h"
#include <stddef.h>

const logging_storage_t *memory_storage_init(void *storage, size_t storage_size);
void memory_storage_deinit(void);
void memory_storage_write_hook(const void *data, size_t size);

#if defined(__cplusplus)
}
#endif

#endif /* MEMORY_STORAGE_H */
