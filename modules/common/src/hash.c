/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/hash.h"

uint32_t hash_murmur_32(const char *key)
{
	uint32_t h = 3323198485;

	while (*key) {
		uint32_t c = (uint32_t)*key++;

		h ^= c;
		h *= 0x5bd1e995;
		h ^= h >> 15;
	}

	return h;
}

uint32_t hash_dbj2_32(const char *key)
{
	uint32_t h = 5381;

	while (*key) {
		uint32_t c = (uint32_t)*key++;

		h = ((h << 5) + h) + c; /* hash * 33 + c */
	}

	return h;
}
