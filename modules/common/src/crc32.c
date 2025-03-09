/*
 * SPDX-FileCopyrightText: 2025 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/crc32.h"

static uint32_t update(uint32_t poly, uint32_t crc, uint8_t c)
{
	crc = crc ^ (uint32_t)(c << 24);

	for (int i = 0; i < 8; i++) {
		if (crc & 0x80000000UL) {
			crc = (crc << 1) ^ poly;
		} else {
			crc <<= 1;
		}
	}

	return crc;
}

static uint32_t update_reverse(uint32_t poly, uint32_t crc, uint8_t c)
{
	crc = crc ^ c;

	for (int i = 0; i < 8; i++) {
		if (crc & 1u) {
			crc = (crc >> 1) ^ poly;
		} else {
			crc = (crc >> 1);
		}
	}

	return crc;
}

uint32_t crc32_compute_table(const uint32_t table[256],
		const uint8_t *data, size_t datasize, uint32_t init)
{
	uint32_t crc = init;

	for (size_t i = 0; i < datasize; i++) {
		crc = (crc << 8) ^ table[(crc >> 24) ^ data[i]];
	}

	return crc;
}

void crc32_generate_table(uint32_t poly, uint32_t table[256])
{
	for (uint32_t i = 0; i < 256; i++) {
		uint32_t crc = i << 24;
		for (int j = 0; j < 8; j++) {
			if (crc & 0x80000000UL) {
				crc = (crc << 1) ^ poly;
			} else {
				crc = (crc << 1);
			}
		}
		table[i] = crc;
	}
}

uint32_t crc32_compute_reverse_table(const uint32_t table[256],
		const uint8_t *data, size_t datasize, uint32_t init)
{
	uint32_t crc = init;

	for (size_t i = 0; i < datasize; i++) {
		crc = (crc >> 8) ^ table[(crc ^ data[i]) & 0xFF];
	}

	return crc;
}

void crc32_generate_reverse_table(uint32_t poly, uint32_t table[256])
{
	for (uint32_t i = 0; i < 256; i++) {
		uint32_t crc = i;
		for (int j = 0; j < 8; j++) {
			if (crc & 1u) {
				crc = (crc >> 1) ^ poly;
			} else {
				crc = (crc >> 1);
			}
		}
		table[i] = crc;
	}
}

uint32_t crc32_jamcrc(const uint8_t *data, size_t datasize)
{
	uint32_t crc = 0xFFFFFFFF;

	for (size_t i = 0; i < datasize; i++) {
		crc = update_reverse(0xEDB88320, crc, data[i]);
	}

	return crc;
}

uint32_t crc32_cksum(const uint8_t *data, size_t datasize)
{
	uint32_t crc = 0;

	for (size_t i = 0; i < datasize; i++) {
		crc = update(0x04C11DB7, crc, data[i]);
	}

	return crc ^ 0xFFFFFFFF;
}
