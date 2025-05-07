/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BASE64_H
#define LIBMCU_BASE64_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

size_t lm_base64_encode(char *buf, size_t bufsize,
		const void *data, size_t datasize);
size_t lm_base64_decode(void *buf, size_t bufsize,
		const char *str, size_t strsize);
size_t lm_base64_decode_overwrite(char *inout, size_t input_size,
		size_t maxlen);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BASE64_H */
