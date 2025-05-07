/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/base64.h"

#include <stdint.h>
#include <stdbool.h>

static char to_base64(uint32_t word, int offset)
{
	const char *tbl =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	return tbl[(word >> (offset * 6)) & 0x3fU];
}

static uint8_t from_base64(char c)
{
	if (c >= 'A' && c <= 'Z') {
		return (uint8_t)(c - 'A');
	} else if (c >= 'a' && c <= 'z') {
		return (uint8_t)(c - 'a' + 26);
	} else if (c >= '0' && c <= '9') {
		return (uint8_t)(c - '0' + 52);
	} else if (c == '+') {
		return 62;
	} else if (c == '/') {
		return 63;
	}
	return 64;
}

static uint8_t from_word(uint32_t word, int offset)
{
	return (word >> (offset * 8)) & 0xffU;
}

static size_t decode(uint8_t *out, const char *in, size_t insize, size_t maxlen)
{
	size_t outdex = 0;

	for (size_t i = 0; i < insize && (outdex+3) <= maxlen; i += 4) {
		const bool e1 = (in[i+2] == '=');
		const bool e2 = (in[i+3] == '=');

		const uint32_t b1 = from_base64(in[i]);
		const uint32_t b2 = from_base64(in[i+1]);
		const uint32_t b3 = from_base64(in[i+2]);
		const uint32_t b4 = from_base64(in[i+3]);
		const uint32_t word = (b1 << 18) | (b2 << 12) | (b3 << 6) | b4;

		out[outdex++] = from_word(word, 2);
		if (e1) break;
		out[outdex++] = from_word(word, 1);
		if (e2) break;
		out[outdex++] = from_word(word, 0);
	}

	return outdex;
}

size_t lm_base64_encode(char *buf, size_t bufsize,
		const void *data, size_t datasize)
{
	const uint8_t *in = (const uint8_t *)data;
	size_t outdex = 0;

	for (size_t i = 0; i < datasize && (outdex+4) <= bufsize; i += 3) {
		const bool c1 = (i+1) < datasize;
		const bool c2 = (i+2) < datasize;

		const uint32_t b1 = in[i];
		const uint32_t b2 = c1? in[i+1] : 0;
		const uint32_t b3 = c2? in[i+2] : 0;
		const uint32_t word = (b1 << 16) | (b2 << 8) | b3;

		buf[outdex++] = to_base64(word, 3);
		buf[outdex++] = to_base64(word, 2);
		buf[outdex++] = c1? to_base64(word, 1) : '=';
		buf[outdex++] = c2? to_base64(word, 0) : '=';
	}

	return outdex;
}

size_t lm_base64_decode(void *buf, size_t bufsize,
		const char *str, size_t strsize)
{
	return decode((uint8_t *)buf, str, strsize, bufsize);
}

size_t lm_base64_decode_overwrite(char *inout, size_t input_size,
		size_t maxlen)
{
	return decode((uint8_t *)inout, inout, input_size, maxlen);
}
