#include "libmcu/base64.h"

#include <stdint.h>
#include <stdbool.h>

static char to_base64(uint32_t word, int offset)
{
	const char *tbl =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	return tbl[(word >> (offset * 6)) & 0x3fu];
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
	return (word >> (offset * 8)) & 0xffu;
}

static size_t decode(uint8_t *out, const char *in, size_t insize)
{
	size_t outdex = 0;

	for (size_t i = 0; i < insize; i += 4) {
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

size_t base64_decode(void *out, const char *in, size_t input_size)
{
	return decode((uint8_t *)out, in, input_size);
}

size_t base64_decode_overwrite(char *inout, size_t input_size)
{
	return decode((uint8_t *)inout, inout, input_size);
}

size_t base64_encode(char *out, const void *data, size_t datasize)
{
	const uint8_t *in = (const uint8_t *)data;
	size_t outdex = 0;

	for (size_t i = 0; i < datasize; i += 3) {
		const bool c1 = (i+1) < datasize;
		const bool c2 = (i+2) < datasize;

		const uint32_t b1 = in[i];
		const uint32_t b2 = c1? in[i+1] : 0;
		const uint32_t b3 = c2? in[i+2] : 0;
		const uint32_t word = (b1 << 16) | (b2 << 8) | b3;

		out[outdex++] = to_base64(word, 3);
		out[outdex++] = to_base64(word, 2);
		out[outdex++] = c1? to_base64(word, 1) : '=';
		out[outdex++] = c2? to_base64(word, 0) : '=';
	}

	return outdex;
}
