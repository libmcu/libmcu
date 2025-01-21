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

#if !defined(base64_encode)
#define base64_encode		libmcu_base64_encode
#endif
#if !defined(base64_decode)
#define base64_decode		libmcu_base64_decode
#endif
#if !defined(base64_decode_overwrite)
#define base64_decode_overwrite	libmcu_base64_decode_overwrite
#endif

size_t base64_encode(char *buf, const void *data, size_t datasize);
size_t base64_decode(void *buf, const char *str, size_t strsize);
size_t base64_decode_overwrite(char *inout, size_t input_size);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BASE64_H */
