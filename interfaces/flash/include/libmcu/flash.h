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

struct lm_flash;

struct lm_flash_api {
	int (*erase)(struct lm_flash *self, uintptr_t offset, size_t size);
	int (*write)(struct lm_flash *self,
			uintptr_t offset, const void *data, size_t len);
	int (*read)(struct lm_flash *self,
			uintptr_t offset, void *buf, size_t len);
	size_t (*size)(struct lm_flash *self);
};

static inline int lm_flash_erase(struct lm_flash *self,
		uintptr_t offset, size_t size) {
	return ((struct lm_flash_api *)self)->erase(self, offset, size);
}

static inline int lm_flash_write(struct lm_flash *self,
		uintptr_t offset, const void *data, size_t len) {
	return ((struct lm_flash_api *)self)->write(self, offset, data, len);
}

static inline int lm_flash_read(struct lm_flash *self,
		uintptr_t offset, void *buf, size_t len) {
	return ((struct lm_flash_api *)self)->read(self, offset, buf, len);
}

static inline size_t lm_flash_size(struct lm_flash *self) {
	return ((struct lm_flash_api *)self)->size(self);
}

struct lm_flash *lm_flash_create(int partition);
void lm_flash_destroy(struct lm_flash *self);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_FLASH_H */
