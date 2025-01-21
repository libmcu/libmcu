/*
 * SPDX-FileCopyrightText: 2018 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/crc16.h"

static uint16_t update(uint16_t poly, uint16_t crc, uint8_t c)
{
	crc = (uint16_t)(crc ^ c);

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
	return update(poly, crc, c);
}

uint16_t crc16_modbus(const void *data, size_t datasize)
{
	const uint8_t *p = (const uint8_t *)data;
	uint16_t crc = 0xFFFFu;

	for (size_t i = 0; i < datasize; i++) {
		crc = update(0xA001, crc, p[i]);
	}

	return crc;
}
