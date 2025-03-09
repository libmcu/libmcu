/*
 * SPDX-FileCopyrightText: 2018 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/crc16.h"

static uint16_t update(uint16_t poly, uint16_t crc, uint8_t c)
{
	crc ^= (uint16_t)c << 8;

	for (int i = 0; i < 8; i++) {
		if (crc & 0x8000U) {
			crc = (uint16_t)(crc << 1) ^ poly;
		} else {
			crc <<= 1;
		}
	}

	return crc;
}

static uint16_t update_reverse(uint16_t poly, uint16_t crc, uint8_t c)
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

uint16_t crc16_update(uint16_t poly, uint16_t crc, uint8_t c)
{
	return update_reverse(poly, crc, c);
}

uint16_t crc16_modbus(const void *data, size_t datasize)
{
	const uint8_t *p = (const uint8_t *)data;
	uint16_t crc = 0xFFFFU;

	for (size_t i = 0; i < datasize; i++) {
		crc = update_reverse(0xA001, crc, p[i]);
	}

	return crc;
}

uint16_t crc16_xmodem(const void *data, size_t datasize)
{
	const uint8_t *p = (const uint8_t *)data;
	uint16_t crc = 0;

	for (size_t i = 0; i < datasize; i++) {
		crc = update(0x1021, crc, p[i]);
	}

	return crc;
}
