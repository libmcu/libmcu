/*
 * SPDX-FileCopyrightText: 2018 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_CRC16_H
#define LIBMCU_CRC16_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

uint16_t crc16_update(uint16_t poly, uint16_t crc, uint8_t c);

uint16_t crc16_modbus(const void *data, size_t datasize);
uint16_t crc16_xmodem(const void *data, size_t datasize);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CRC16_H */
