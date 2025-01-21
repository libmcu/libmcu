/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_FLASH_H
#define LIBMCU_FLASH_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

struct flash;

struct flash_api {
	int (*erase)(struct flash *self, uintptr_t offset, size_t size);
	int (*write)(struct flash *self,
			uintptr_t offset, const void *data, size_t len);
	int (*read)(struct flash *self,
			uintptr_t offset, void *buf, size_t len);
	size_t (*size)(struct flash *self);
};

static inline int flash_erase(struct flash *self,
		uintptr_t offset, size_t size) {
	return ((struct flash_api *)self)->erase(self, offset, size);
}

static inline int flash_write(struct flash *self,
		uintptr_t offset, const void *data, size_t len) {
	return ((struct flash_api *)self)->write(self, offset, data, len);
}

static inline int flash_read(struct flash *self,
		uintptr_t offset, void *buf, size_t len) {
	return ((struct flash_api *)self)->read(self, offset, buf, len);
}

static inline size_t flash_size(struct flash *self) {
	return ((struct flash_api *)self)->size(self);
}

struct flash *flash_create(int partition);
int flash_delete(struct flash);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_FLASH_H */
