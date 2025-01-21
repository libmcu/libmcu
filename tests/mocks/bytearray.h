/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BYTEARRAY_H
#define LIBMCU_BYTEARRAY_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <string.h>
#include "CppUTestExt/MockSupport.h"

#if !defined(MIN)
#define MIN(a, b)		(((a) > (b))? (b) : (a))
#endif

struct ByteArray {
	const void *ptr;
	size_t len;
};

class ByteArrayComparator : public MockNamedValueComparator {
	public:
		virtual bool isEqual(const void *object1, const void *object2) {
			const struct ByteArray *p1 = (const struct ByteArray *)object1;
			const struct ByteArray *p2 = (const struct ByteArray *)object2;

			if (p1->len != p2->len) {
				return false;
			}

			return memcmp(p1->ptr, p2->ptr, MIN(p1->len, p2->len)) == 0;
		}
		virtual SimpleString valueToString(const void *object) {
			return StringFrom(object);
		}
};

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BYTEARRAY_H */
