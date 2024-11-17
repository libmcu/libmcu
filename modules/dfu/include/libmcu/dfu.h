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
} dfu_error_t;

typedef enum {
	DFU_TYPE_BOOTLOADER		= 1,
	DFU_TYPE_LOADER			= 2,
	DFU_TYPE_UPDATOR		= 3,
	DFU_TYPE_APP			= 4,
} dfu_type_t;

typedef enum {
	DFU_HEADER_VERSION_1		= 1,
	DFU_HEADER_VERSION_CURRENT	= DFU_HEADER_VERSION_1,
} dfu_header_version_t;

typedef enum {
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

dfu_error_t dfu_init(void);
void dfu_activate(void);
void dfu_deactivate(void);
bool dfu_is_activated(void);
int dfu_get_reboot_count(void);
void dfu_increment_reboot_count(void);
void dfu_clear_reboot_count(void);
dfu_error_t dfu_write(dfu_slot_t slot, uint32_t offset,
		const void *data, size_t datasize);
bool dfu_is_image_valid(dfu_slot_t slot, const struct dfu_image_header *header);
dfu_error_t dfu_invalidate(dfu_slot_t slot);
dfu_error_t dfu_commit(dfu_slot_t slot, const struct dfu_image_header *header);
dfu_error_t dfu_get_slot_addr(dfu_slot_t slot, uintptr_t *addr);
void dfu_jump(const struct dfu_image_header *header);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_DFU_H */
