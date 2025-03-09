/*
 * SPDX-FileCopyrightText: 2025 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_CRC32_H
#define LIBMCU_CRC32_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Computes the CRC-32 value using a precomputed table.
 *
 * @param[in] table The precomputed CRC-32 table.
 * @param[in] data Pointer to the data to compute the CRC for.
 * @param[in] datasize Size of the data in bytes.
 * @param[in] init Initial CRC value.
 *
 * @return The computed CRC-32 value.
 */
uint32_t crc32_compute_table(const uint32_t table[256],
		const uint8_t *data, size_t datasize, uint32_t init);

/**
 * @brief Generates a CRC-32 table using the specified polynomial.
 *
 * @param[in] poly The polynomial to use for generating the table.
 * @param[out] table The generated CRC-32 table.
 */
void crc32_generate_table(uint32_t poly, uint32_t table[256]);

/**
 * @brief Computes the CRC-32 value using a precomputed reverse table.
 *
 * @param[in] table The precomputed reverse CRC-32 table.
 * @param[in] data Pointer to the data to compute the CRC for.
 * @param[in] datasize Size of the data in bytes.
 * @param[in] init Initial CRC value.
 *
 * @return The computed CRC-32 value.
 */
uint32_t crc32_compute_reverse_table(const uint32_t table[256],
		const uint8_t *data, size_t datasize, uint32_t init);

/**
 * @brief Generates a reverse CRC-32 table using the specified polynomial.
 *
 * @param[in] poly The polynomial to use for generating the reverse table.
 * @param[out] table The generated reverse CRC-32 table.
 */
void crc32_generate_reverse_table(uint32_t poly, uint32_t table[256]);

/**
 * @brief Computes the CRC-32/JAMCRC checksum for the given data.
 *
 * @param[in] data Pointer to the data to compute the checksum for.
 * @param[in] datasize Size of the data in bytes.
 *
 * @return The computed CRC-32/JAMCRC checksum.
 */
uint32_t crc32_jamcrc(const uint8_t *data, size_t datasize);

/**
 * @brief Computes the CRC-32/CKSUM checksum for the given data.
 *
 * @param[in] data Pointer to the data to compute the checksum for.
 * @param[in] datasize Size of the data in bytes.
 *
 * @return The computed CRC-32/CKSUM checksum.
 */
uint32_t crc32_cksum(const uint8_t *data, size_t datasize);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_CRC32_H */
