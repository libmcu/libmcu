/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/flash_kvstore.h"
#include "libmcu/compiler.h"
#include "libmcu/hash.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>

#if !defined(MIN)
#define MIN(a, b)			((a) > (b)? (b) : (a))
#endif

struct meta_entry {
	uint32_t hash_murmur;
	uint32_t hash_dbj2;
	uint32_t offset;
	uint32_t len;
} LIBMCU_PACKED;

struct meta {
	struct meta_entry entry;
	uintptr_t offset;
};

struct subsector {
	uintptr_t offset;
	size_t size;
};

struct storage {
	struct subsector meta;
	struct subsector data;

	size_t size;

	struct flash *flash;
	struct flash *flash_scratch;
};

struct kvstore {
	struct kvstore_api api;

	struct storage storage;
};

static bool is_free_entry(struct meta_entry *entry)
{
	if (entry->hash_dbj2 == 0xffffffff &&
			entry->hash_murmur == 0xffffffff) {
		return true;
	}

	return false;
}

static int find_key(struct kvstore *self, const char *key, struct meta *meta)
{
	uint32_t hash_murmur = hash_murmur_32(key);
	uint32_t hash_dbj2 = hash_dbj2_32(key);

	uintptr_t start = self->storage.meta.offset;
	uintptr_t end = start + self->storage.meta.size;
	uint32_t entry_size = sizeof(struct meta_entry);

	for (uintptr_t offset = start; offset < end; offset += entry_size) {
		if (flash_read(self->storage.flash,
				offset, &meta->entry, entry_size) < 0) {
			return -EIO;
		}

		if (hash_murmur == meta->entry.hash_murmur &&
				hash_dbj2 == meta->entry.hash_dbj2) {
			meta->offset = offset;
			return 0;
		}
	}

	return -ENOENT;
}

static int alloc_entry(struct kvstore *self, size_t size, struct meta *meta)
{
	uintptr_t start = self->storage.meta.offset;
	uintptr_t end = start + self->storage.meta.size;
	uint32_t entry_size = sizeof(struct meta_entry);
	uint32_t new_data_offset = 0;
	bool allocated = false;

	for (uintptr_t offset = start; offset < end; offset += entry_size) {
		if (flash_read(self->storage.flash,
				offset, &meta->entry, entry_size) < 0) {
			return -EIO;
		}

		if (!allocated && is_free_entry(&meta->entry)) {
			meta->offset = offset;
			allocated = true;
		} else {
			uint32_t t = meta->entry.offset + meta->entry.len;
			if (t > new_data_offset && t < self->storage.data.size) {
				new_data_offset = t;
			}
		}
	}

	if (allocated && (new_data_offset + size) < self->storage.data.size) {
		meta->entry.offset = new_data_offset;
		meta->entry.len = (uint32_t)size;
		return 0;
	}

	return -ENOSPC;
}

static int delete_meta(struct kvstore *self, struct meta *meta)
{
	struct meta_entry *entry = &meta->entry;

	entry->hash_murmur = 0;
	entry->hash_dbj2 = 0;

	return flash_write(self->storage.flash,
			meta->offset, entry, sizeof(*entry));
}

static int write_meta(struct kvstore *self, const struct meta *meta)
{
	return flash_write(self->storage.flash,
			meta->offset, &meta->entry, sizeof(meta->entry));
}

static int write_value(struct kvstore *self,
		const void *data, const struct meta *meta)
{
	uintptr_t offset = self->storage.meta.size + meta->entry.offset;
	return flash_write(self->storage.flash, offset, data, meta->entry.len);
}

/* TODO: implement get_available_bytes() */
static size_t get_available_bytes(struct kvstore *self)
{
	unused(self);
	return 0;
}

/* TODO: implement reclaim() */
static int reclaim(struct kvstore *self)
{
	unused(self);
	return 0;
}

static int flash_kvstore_write(struct kvstore *self,
		const char *key, const void *value, size_t size)
{
	struct meta new_meta;
	struct meta old_meta;

	if (alloc_entry(self, size, &new_meta) != 0) {
		if (get_available_bytes(self) < size) {
			return -ENOSPC;
		}

		if (reclaim(self) != 0 ||
				alloc_entry(self, size, &new_meta) != 0) {
			return -EFAULT;
		}
	}

	if (find_key(self, key, &old_meta) == 0) {
		delete_meta(self, &old_meta);
	}

	new_meta.entry.hash_murmur = hash_murmur_32(key);
	new_meta.entry.hash_dbj2 = hash_dbj2_32(key);

	if (write_meta(self, &new_meta) != 0 ||
			write_value(self, value, &new_meta) != 0) {
		return -EIO;
	}

	return 0;
}

static int flash_kvstore_read(struct kvstore *self,
		const char *key, void *buf, size_t size)
{
	struct meta meta;

	if (find_key(self, key, &meta) != 0) {
		return -ENOENT;
	}

	size_t len = MIN(size, meta.entry.len);
	uintptr_t offset = self->storage.meta.size + meta.entry.offset;
	return flash_read(self->storage.flash, offset, buf, len);
}

static int flash_kvstore_erase(struct kvstore *self, const char *key)
{
	struct meta meta;

	if (find_key(self, key, &meta) == 0) {
		return delete_meta(self, &meta);
	}

	return -ENOENT;
}

static int flash_kvstore_open(struct kvstore *self, const char *namespace)
{
	/* WARN: only one namespace supported */
	unused(self);
	unused(namespace);
	return 0;
}

static void flash_kvstore_close(struct kvstore *self)
{
	unused(self);
}

struct kvstore *flash_kvstore_new(struct flash *flash, struct flash *scratch)
{
	/* WARN: single instance */
	static struct kvstore nvs = {
		.api = {
			.write = flash_kvstore_write,
			.read = flash_kvstore_read,
			.clear = flash_kvstore_erase,
			.open = flash_kvstore_open,
			.close = flash_kvstore_close,
		},
	};

	nvs.storage.flash = flash;
	nvs.storage.flash_scratch = scratch;
	nvs.storage.size = flash_size(flash);

	/* meta takes about 6% of the given space */
	size_t meta_size = nvs.storage.size >> 4;
	size_t data_size = nvs.storage.size - meta_size;
	uintptr_t meta_offset = 0;
	uintptr_t data_offset = 0;

	nvs.storage.meta.offset = meta_offset;
	nvs.storage.meta.size = meta_size;
	nvs.storage.data.offset = data_offset;
	nvs.storage.data.size = data_size;

	return &nvs;
}
