/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_COBS_H
#define LIBMCU_COBS_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

size_t cobs_encode(uint8_t *buf, size_t bufsize,
		   void const *data, size_t datasize);
size_t cobs_decode(uint8_t *buf, size_t bufsize,
		   uint8_t const *data, size_t datasize);
size_t cobs_decode_overwrite(uint8_t *inout, size_t maxlen);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_COBS_H */
