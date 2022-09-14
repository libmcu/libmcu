/* https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing */

#include "libmcu/cobs.h"

#if !defined(MIN)
#define MIN(a, b)		(((a) > (b))? (b) : (a))
#endif

size_t cobs_encode(uint8_t *buf, size_t bufsize,
		   void const *data, size_t datasize)
{
	uint8_t const *p = (uint8_t const *)data;
	size_t group_head_index = 0;
	uint8_t group_len = 1;
	size_t maxlen = MIN(bufsize, datasize);
	size_t o = 1; /* outdex */

	for (size_t i = 0; i < maxlen && o < bufsize; i++) {
		if (p[i] != 0) {
			buf[o++] = p[i];
			group_len++;
		}

		if (p[i] == 0 || group_len == 0xFFu) {
			buf[group_head_index] = group_len;
			group_head_index = o++;
			if (o >= bufsize ||
				(group_len == 0xFFu && (i+1) >= maxlen)) {
				buf[MIN(o - 1, bufsize - 1)] = 0;
				return o - 1;
			}
			group_len = 1;
		}

	}

	buf[group_head_index] = group_len;
	buf[MIN(o, bufsize - 1)] = 0;

	return o;
}

size_t cobs_decode(uint8_t *buf, size_t bufsize,
		   uint8_t const *data, size_t datasize)
{
	size_t o = 0;
	uint8_t group_len = 0;
	uint8_t code = 0xff;

	for (size_t i = 0; i < datasize && o < bufsize; i++) {
		if (group_len > 0) {
			buf[o++] = data[i];
		} else {
			if (code != 0xff) {
				buf[o++] = 0;
			}

			group_len = code = data[i];

			if (code == 0) {
				break;
			}
		}

		group_len--;
	}

	return o;
}

size_t cobs_decode_overwrite(uint8_t *inout, size_t maxlen)
{
	return cobs_decode(inout, maxlen, inout, maxlen);
}
