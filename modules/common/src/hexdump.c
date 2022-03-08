#include "libmcu/hexdump.h"
#include <stdint.h>

#if !defined(HEXDUMP_WIDTH)
#define HEXDUMP_WIDTH				16
#endif
#if !defined(HEXDUMP_ADDRLEN)
#define HEXDUMP_ADDRLEN				8
#endif

#if !defined(MIN)
#define MIN(a, b)				(((a) > (b))? (b) : (a))
#endif

#define LINE_LENGTH				\
	(HEXDUMP_ADDRLEN + 1/*:*/ + 3 * HEXDUMP_WIDTH + 1/*\t*/ + \
	 HEXDUMP_WIDTH + 1/*\n*/)

static size_t get_hexstr_reversed(char *buf, size_t bufsize, uint32_t value,
		size_t width, char c_padding)
{
	const char *hextbl = "0123456789abcdef";
	size_t len;

	for (len = 0; value != 0 && len < bufsize; len++) {
		buf[len] = hextbl[value & 0xf];
		value >>= 4;
	}

	if (len == 0) {
		buf[0] = '0';
		len = 1;
	}

	for (size_t i = len; i < width && i < (bufsize - 1); i++) {
		buf[i] = c_padding;
	}

	buf[width] = '\0';

	return width;
}

static size_t print_address(uint8_t *buf, size_t bufsize, uintptr_t addr)
{
	char c[HEXDUMP_ADDRLEN + 1];
	size_t len = get_hexstr_reversed(c, sizeof(c), (uint32_t)addr,
			HEXDUMP_ADDRLEN, ' ');

	for (size_t i = 0; i < len && i < (bufsize - 2); i++) {
		buf[i] = (uint8_t)c[len - i - 1];
	}

	buf[len++] = ':';

	if (len > 0) {
		return len;
	}

	return 0;
}

static size_t print_hex(uint8_t *buf, size_t bufsize,
		const uint8_t *data, size_t datasize)
{
	char c[3];
	size_t buf_i = 0;

	for (size_t data_i = 0; data_i < datasize
			&& (buf_i + 4) < bufsize; data_i++) {
		get_hexstr_reversed(c, sizeof(c), data[data_i], 2, '0');

		buf[buf_i++] = ' ';
		buf[buf_i++] = (uint8_t)c[1];
		buf[buf_i++] = (uint8_t)c[0];
	}

	for (size_t i = datasize; i < HEXDUMP_WIDTH
			&& (buf_i + 4) < bufsize; i++) {
		buf[buf_i++] = ' ';
		buf[buf_i++] = ' ';
		buf[buf_i++] = ' ';
	}

	return buf_i;
}

static size_t print_ascii(uint8_t *buf, size_t bufsize,
		const uint8_t *data, size_t datasize)
{
	size_t buf_i = 0;
	size_t skipped = 0;

	buf[buf_i++] = '\t';

	for (size_t i = 0; i < HEXDUMP_WIDTH && buf_i < (bufsize - 2); i++) {
		if (i >= datasize) {
			skipped += HEXDUMP_WIDTH - i;
			break;
		}

		if (data[i] >= 0x20 && data[i] < 0x7F) {
			buf[buf_i++] = data[i];
		} else {
			buf[buf_i++] = '.';
		}
	}

	buf[buf_i++] = '\n';
	buf[buf_i] = '\0';

	return buf_i + skipped;
}

size_t hexdump(void *buf, size_t bufsize, void const *data, size_t datasize)
{
	if (buf == NULL || data == NULL || datasize == 0 || bufsize == 0) {
		return 0;
	}

	char const *src = (char const *)data;
	char *dst = (char *)buf;
	size_t outdex = 0;
	char c[3];

	for (size_t i = 0; i < datasize && (outdex + 4) < bufsize; i++) {
		get_hexstr_reversed(c, sizeof(c), (uint32_t)src[i], 2, '0');
		dst[outdex++] = c[1];
		dst[outdex++] = c[0];
	}

	dst[outdex] = '\0';

	return outdex;
}

size_t hexdump_verbose(void *buf, size_t bufsize,
		void const *data, size_t datasize)
{
	if (buf == NULL || data == NULL || datasize == 0 || bufsize == 0) {
		return 0;
	}

	uint8_t *dst = (uint8_t *)buf;
	const uint8_t *src = (const uint8_t *)data;
	size_t buf_i = 0;

	for (size_t data_i = 0; data_i < datasize
			&& (buf_i + LINE_LENGTH + 1) < bufsize;
			data_i += HEXDUMP_WIDTH) {
		size_t len = MIN(HEXDUMP_WIDTH, datasize - data_i);
		buf_i += print_address(&dst[buf_i], bufsize - buf_i, data_i);
		buf_i += print_hex(&dst[buf_i], bufsize - buf_i,
				&src[data_i], len);
		buf_i += print_ascii(&dst[buf_i], bufsize - buf_i,
				&src[data_i], len);
	}

	dst[buf_i] = '\0';

	return buf_i;
}

size_t hexdump_compute_output_size(size_t datasize)
{
	size_t n = datasize / HEXDUMP_WIDTH;
	n += (datasize % HEXDUMP_WIDTH) != 0? 1 : 0;
	return n * LINE_LENGTH;
}
