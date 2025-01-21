/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_HASH_H
#define LIBMCU_HASH_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

uint32_t hash_murmur_32(const char *key);
uint32_t hash_dbj2_32(const char *key);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_HASH_H */
