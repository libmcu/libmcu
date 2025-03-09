/*
 * SPDX-FileCopyrightText: 2018 권경환 Kyunghwan Kwon <k@libmcu.org>
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

/**
 * @brief Updates the CRC-16 value with new data.
 *
 * @param[in] poly The polynomial to use for the CRC calculation.
 * @param[in] crc The current CRC value.
 * @param[in] c The new data byte to update the CRC with.
 *
 * @return The updated CRC value.
 */
uint16_t crc16_update(uint16_t poly, uint16_t crc, uint8_t c);

/**
 * @brief Computes the CRC-16/MODBUS checksum for the given data.
 *
 * @param[in] data Pointer to the data to compute the checksum for.
 * @param[in] datasize Size of the data in bytes.
 *
 * @return The computed CRC-16/MODBUS checksum.
 */
uint16_t crc16_modbus(const void *data, size_t datasize);

/**
 * @brief Computes the CRC-16/XMODEM checksum for the given data.
 *
 * @param[in] data Pointer to the data to compute the checksum for.
 * @param[in] datasize Size of the data in bytes.
 *
 * @return The computed CRC-16/XMODEM checksum.
 */
uint16_t crc16_xmodem(const void *data, size_t datasize);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CRC16_H */
