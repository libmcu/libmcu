/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_DFU_H
#define LIBMCU_DFU_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if !defined(DFU_MAGIC)
#define DFU_MAGIC			0xC0DEu
#endif

typedef enum {
	DFU_ERROR_NONE			= 0,
	DFU_ERROR_IO			= -1,
	DFU_ERROR_ALLOC_FAIL		= -2,
	DFU_ERROR_SLOT_UPDATE_FAIL	= -3,
	DFU_ERROR_INVALID_SLOT		= -4,
	DFU_ERROR_INVALID_IMAGE		= -5,
	DFU_ERROR_INVALID_SIGNATURE	= -6,
	DFU_ERROR_INVALID_PARAM		= -7,
	DFU_ERROR_INVALID_VERSION	= -8,
	DFU_ERROR_UNSUPPORT_VERSOIN	= -9,
	DFU_ERROR_NOT_IMPLEMENTED	= -10,
} dfu_error_t;

typedef enum {
	DFU_TYPE_BOOTLOADER		= 1,
	DFU_TYPE_LOADER			= 2,
	DFU_TYPE_UPDATOR		= 3,
	DFU_TYPE_APP			= 4,
} dfu_type_t;

typedef enum {
	DFU_SLOT_CURRENT		= 0,
	DFU_SLOT_1			= 1,
	DFU_SLOT_2			= 2,
	DFU_SLOT_3			= 3,
	DFU_SLOT_4			= 4,
	DFU_SLOT_MAX			= DFU_SLOT_4,
} dfu_slot_t;

struct dfu_image_header {
	uint16_t magic;
	uint16_t header_version;
	uint32_t datasize;
	uint8_t type; /* image type */
	uint8_t version_major; /* image version */
	uint8_t version_minor;
	uint8_t version_patch;
	uint32_t vector_addr; /* vector table address */
	uint8_t git_sha[8];
	uint8_t signature[64];
	uint8_t iv_nonce[16];
} __attribute__((packed));

struct dfu;

/**
 * @brief Creates a new DFU (Device Firmware Update) instance.
 *
 * @param[in] data_block_size The size of the data block to be used for the DFU
 *                            process.
 *
 * @return A pointer to the newly created DFU instance.
 */
struct dfu *dfu_new(size_t data_block_size);

/**
 * @brief Deletes a DFU instance.
 *
 * @param[in] dfu A pointer to the DFU instance to be deleted.
 */
void dfu_delete(struct dfu *dfu);

/**
 * @brief Prepares the DFU instance with the provided image header.
 *
 * @param[in,out] dfu A pointer to the DFU instance.
 * @param[in] header A pointer to the DFU image header.
 *
 * @return A dfu_error_t indicating the result of the operation.
 */
dfu_error_t dfu_prepare(struct dfu *dfu, const struct dfu_image_header *header);

/**
 * @brief Writes data to the DFU instance at the specified offset.
 *
 * @param[in,out] dfu A pointer to the DFU instance.
 * @param[in] offset The offset at which to write the data.
 * @param[in] data A pointer to the data to be written.
 * @param[in] datasize The size of the data to be written.
 *
 * @return A dfu_error_t indicating the result of the operation.
 */
dfu_error_t dfu_write(struct dfu *dfu, uint32_t offset,
		const void *data, size_t datasize);

/**
 * @brief Finishes the DFU process.
 *
 * @param[in,out] dfu A pointer to the DFU instance.
 *
 * @return A dfu_error_t indicating the result of the operation.
 */
dfu_error_t dfu_finish(struct dfu *dfu);

/**
 * @brief Aborts the ongoing DFU operation.
 *
 * This function aborts the current DFU operation for the given DFU instance.
 * It stops any ongoing update tasks and cleans up any resources associated with
 * the update.
 *
 * @param[in] dfu Pointer to the DFU instance for which the update is to be
 *            aborted.
 *
 * @return A dfu_error_t indicating the result of the operation.
 */
dfu_error_t dfu_abort(struct dfu *dfu);

/**
 * @brief Commits the DFU process, making the update permanent.
 *
 * @param[in,out] dfu A pointer to the DFU instance.
 *
 * @return A dfu_error_t indicating the result of the operation.
 */
dfu_error_t dfu_commit(struct dfu *dfu);

/**
 * @brief Checks if the provided image header is valid.
 *
 * @param[in] header A pointer to the DFU image header.
 *
 * @return true if the header is valid, false otherwise.
 */
bool dfu_is_valid_header(const struct dfu_image_header *header);

/**
 * @brief Invalidates the specified DFU slot.
 *
 * @param[in] slot The DFU slot to be invalidated.
 *
 * @return A dfu_error_t indicating the result of the operation.
 */
dfu_error_t dfu_invalidate(dfu_slot_t slot);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_DFU_H */
