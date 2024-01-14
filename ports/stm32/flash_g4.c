/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/flash.h"

#include <errno.h>
#include <string.h>

#include "libmcu/compiler.h"
#include "libmcu/logging.h"

#if !defined(STM32_NVS_FLASH_ADDR)
#define STM32_NVS_FLASH_ADDR		0x0807f000 /* bank2 page.126 */
#endif
#if !defined(STM32_NVS_FLASH_SECTOR_SIZE)
#define STM32_NVS_FLASH_SECTOR_SIZE	(2 * 1024)
#endif
#if !defined(STM32_NVS_FLASH_SIZE)
#define STM32_NVS_FLASH_SIZE		STM32_NVS_FLASH_SECTOR_SIZE
#endif
#if !defined(STM32_NVS_SCRATCH_FLASH_ADDR)
#define STM32_NVS_SCRATCH_FLASH_ADDR	0x0807f800 /* bank2 page.127 */
#endif
LIBMCU_ASSERT(STM32_NVS_FLASH_SIZE >= 1024);
LIBMCU_ASSERT(STM32_NVS_FLASH_SECTOR_SIZE >= STM32_NVS_FLASH_SIZE);

struct flash {
	struct flash_api api;
	uintptr_t baseaddr;
	size_t size;
	size_t sector_size;
};

static int do_erase(struct flash *self, uintptr_t offset, size_t size)
{
	int rc = 0;
	uint32_t errorStatus = 0;
	FLASH_EraseInitTypeDef FLASH_EraseInitStruct = {
		.TypeErase = FLASH_TYPEERASE_PAGES,
		.Page = ((self->baseaddr & 0xfffff) + offset) /
			self->sector_size,
		.NbPages = size / self->sector_size,
		.Banks = FLASH_BANK_2,
	};

	HAL_FLASH_Unlock();

	if (HAL_FLASHEx_Erase(&FLASH_EraseInitStruct, &errorStatus) != HAL_OK) {
		error("error erasing flash(%x) %lx",
				errorStatus, self->baseaddr + offset);
		rc = -EIO;
	}

	HAL_FLASH_Lock();

	return rc;
}

static int do_write(struct flash *self,
		uintptr_t offset, const void *data, size_t len)
{
	int rc = 0;
	uint32_t addr = self->baseaddr + offset;
	const uint8_t *p = (const uint8_t *)data;
	uint64_t tmp;

	HAL_FLASH_Unlock();

	for (size_t written = 0; written < len; written += sizeof(tmp)) {
		tmp = 0xffffffffffffffffull;
		if ((written + sizeof(tmp)) <= len) {
			memcpy(&tmp, &p[written], sizeof(tmp));
		} else {
			memcpy(&tmp, &p[written], len - written);
		}

		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
				addr + written, tmp) != HAL_OK) {
			rc = -EIO;
			error("error flashing at %lx(%lx)",
					addr + written, HAL_FLASH_GetError());
			break;
		}
	}

	HAL_FLASH_Lock();

	return rc;
}

static int do_read(struct flash *self, uintptr_t offset, void *buf, size_t len)
{
	uint32_t addr = self->baseaddr + offset;
	uint8_t *p = (uint8_t *)buf;

	for (uint32_t i = 0; i < len; i++) {
		p[i] = *(volatile uint8_t *)(addr + i);
	}

	return (int)len;
}

static size_t do_size(struct flash *self)
{
	return self->size;
}

struct flash *flash_create(int partition)
{
	static struct flash flash[] = {
		{
			.api = {
				.write = do_write,
				.read = do_read,
				.erase = do_erase,
				.size = do_size,
			},

			.baseaddr = STM32_NVS_FLASH_ADDR,
			.size = STM32_NVS_FLASH_SIZE,
			.sector_size = STM32_NVS_FLASH_SECTOR_SIZE,
		},
		{
			.api = {
				.write = do_write,
				.read = do_read,
				.erase = do_erase,
				.size = do_size,
			},

			.baseaddr = STM32_NVS_SCRATCH_FLASH_ADDR,
			.size = STM32_NVS_FLASH_SIZE,
			.sector_size = STM32_NVS_FLASH_SECTOR_SIZE,
		},
	};

	if (partition < 0 || partition > 1) {
		return NULL;
	}

	return &flash[partition];
}
