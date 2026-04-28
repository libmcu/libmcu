/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/base64.h"

#include <stdint.h>
#include <stdbool.h>

static const char ctbl[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char ctbl_url[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static char to_base64_variant(uint32_t word, int offset, const char *tbl)
{
	return tbl[(word >> (offset * 6)) & 0x3fU];
}

static uint8_t from_base64(char c, char code62, char code63)
{
	if (c >= 'A' && c <= 'Z') {
		return (uint8_t)(c - 'A');
	} else if (c >= 'a' && c <= 'z') {
		return (uint8_t)(c - 'a' + 26);
	} else if (c >= '0' && c <= '9') {
		return (uint8_t)(c - '0' + 52);
	} else if (c == code62) {
		return 62;
	} else if (c == code63) {
		return 63;
	}
	return 64;
}

static uint8_t from_word(uint32_t word, int offset)
{
	return (word >> (offset * 8)) & 0xffU;
}

static size_t encode(char *buf, size_t bufsize,
		const uint8_t *in, size_t insize, const char *tbl)
{
	size_t outdex = 0;

	for (size_t i = 0; i < insize && (outdex+4) <= bufsize; i += 3) {
		const bool c1 = (i+1) < insize;
		const bool c2 = (i+2) < insize;

		const uint32_t b1 = in[i];
		const uint32_t b2 = c1? in[i+1] : 0;
		const uint32_t b3 = c2? in[i+2] : 0;
		const uint32_t word = (b1 << 16) | (b2 << 8) | b3;

		buf[outdex++] = to_base64_variant(word, 3, tbl);
		buf[outdex++] = to_base64_variant(word, 2, tbl);
		buf[outdex++] = c1? to_base64_variant(word, 1, tbl) : '=';
		buf[outdex++] = c2? to_base64_variant(word, 0, tbl) : '=';
	}

	return outdex;
}

static size_t decode(uint8_t *out, const char *in, size_t insize, size_t maxlen,
		char c62, char c63)
{
	size_t outdex = 0;

	for (size_t i = 0; (i+1) < insize; i += 4) {
		const bool c1 = (i+2) < insize && in[i+2] != '=';
		const bool c2 = (i+3) < insize && in[i+3] != '=';
		const size_t nbytes = c1? (c2? 3U : 2U) : 1U;

		if ((outdex + nbytes) > maxlen) {
			break;
		}

		const uint32_t b1 = from_base64(in[i], c62, c63);
		const uint32_t b2 = from_base64(in[i+1], c62, c63);
		const uint32_t b3 = c1? from_base64(in[i+2], c62, c63) : 0;
		const uint32_t b4 = c2? from_base64(in[i+3], c62, c63) : 0;
		const uint32_t word = (b1 << 18) | (b2 << 12) | (b3 << 6) | b4;

		out[outdex++] = from_word(word, 2);
		if (!c1) {
			break;
		}
		out[outdex++] = from_word(word, 1);
		if (!c2) {
			break;
		}
		out[outdex++] = from_word(word, 0);
	}

	return outdex;
}

size_t lm_base64_encode(char *buf, size_t bufsize,
		const void *data, size_t datasize)
{
	return encode(buf, bufsize, (const uint8_t *)data, datasize, ctbl);
}

size_t lm_base64url_encode(char *buf, size_t bufsize,
		const void *data, size_t datasize)
{
	return encode(buf, bufsize, (const uint8_t *)data, datasize, ctbl_url);
}

size_t lm_base64_decode(void *buf, size_t bufsize,
		const char *str, size_t strsize)
{
	return decode((uint8_t *)buf, str, strsize, bufsize, '+', '/');
}

size_t lm_base64_decode_overwrite(char *inout, size_t input_size, size_t maxlen)
{
	return decode((uint8_t *)inout, inout, input_size, maxlen, '+', '/');
}

size_t lm_base64url_decode(void *buf, size_t bufsize,
		const char *str, size_t strsize)
{
	return decode((uint8_t *)buf, str, strsize, bufsize, '-', '_');
}

size_t lm_base64url_decode_overwrite(char *inout, size_t input_size,
		size_t maxlen)
{
	return decode((uint8_t *)inout, inout, input_size, maxlen, '-', '_');
}
