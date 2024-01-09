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
#include <string.h>

#if !defined(FLASH_LINE_ALIGN_BYTES)
#define FLASH_LINE_ALIGN_BYTES		16
#endif

#if !defined(MIN)
#define MIN(a, b)			((a) > (b)? (b) : (a))
#endif

struct meta_entry {
	uint32_t hash_murmur;
	uint32_t hash_dbj2;
	uint32_t offset;
	uint32_t len;
	uint8_t _padding[FLASH_LINE_ALIGN_BYTES - 16];
} LIBMCU_PACKED;
LIBMCU_ASSERT(sizeof(struct meta_entry) == FLASH_LINE_ALIGN_BYTES);

struct meta {
	struct meta_entry entry;
	uintptr_t offset;
};

struct subsector {
	uintptr_t offset;
	size_t size;
};

struct storage {
	struct flash *flash;

	struct subsector meta;
	struct subsector data;

	uintptr_t offset;
	size_t size;
};

struct kvstore {
	struct kvstore_api api;

	struct storage storage;
	struct storage scratch;
};

static bool is_free_entry(struct meta_entry *entry)
{
	if (entry->hash_dbj2 == 0xffffffff &&
			entry->hash_murmur == 0xffffffff) {
		return true;
	}

	return false;
}

static int find_key(struct storage *storage, const char *key, struct meta *meta)
{
	uint32_t hash_murmur = hash_murmur_32(key);
	uint32_t hash_dbj2 = hash_dbj2_32(key);

	uintptr_t start = storage->offset + storage->meta.offset;
	uintptr_t end = start + storage->meta.size;
	uint32_t entry_size = sizeof(struct meta_entry);
	int keycnt = 0;

	for (uintptr_t offset = start; offset < end; offset += entry_size) {
		struct meta t;

		if (flash_read(storage->flash,
				offset, &t.entry, entry_size) < 0) {
			return -EIO;
		}

		/* the last one is the latest one */
		if (hash_murmur == t.entry.hash_murmur &&
				hash_dbj2 == t.entry.hash_dbj2) {
			memcpy(&meta->entry, &t.entry, sizeof(t.entry));
			meta->offset = offset;
			keycnt++;
		}
	}

	if (keycnt > 0) {
		return 0;
	}

	return -ENOENT;
}

static int find_meta(struct storage *storage, struct meta *meta)
{
	uintptr_t start = storage->offset + storage->meta.offset;
	uintptr_t end = start + storage->meta.size;
	uint32_t entry_size = sizeof(struct meta_entry);
	int keycnt = 0;

	for (uintptr_t offset = start; offset < end; offset += entry_size) {
		struct meta t;

		if (flash_read(storage->flash,
				offset, &t.entry, entry_size) < 0) {
			return -EIO;
		}

		/* the last one is the latest one */
		if (meta->entry.hash_murmur == t.entry.hash_murmur &&
				meta->entry.hash_dbj2 == t.entry.hash_dbj2) {
			memcpy(&meta->entry, &t.entry, sizeof(t.entry));
			meta->offset = offset;
			keycnt++;
		}
	}

	if (keycnt > 0) {
		return 0;
	}

	return -ENOENT;
}

static int alloc_entry(struct storage *storage, size_t size, struct meta *meta)
{
	uintptr_t start = storage->offset + storage->meta.offset;
	uintptr_t end = start + storage->meta.size;
	uint32_t entry_size = sizeof(struct meta_entry);
	uint32_t new_data_offset = 0;
	bool allocated = false;

	for (uintptr_t offset = start; offset < end; offset += entry_size) {
		if (flash_read(storage->flash,
				offset, &meta->entry, entry_size) < 0) {
			return -EIO;
		}

		if (!allocated && is_free_entry(&meta->entry)) {
			meta->offset = offset;
			allocated = true;
		} else {
			uint32_t t = ALIGN(meta->entry.offset + meta->entry.len,
					FLASH_LINE_ALIGN_BYTES);
			if (t > new_data_offset &&
					t < storage->data.size) {
				new_data_offset = t;
			}
		}
	}

	if (allocated && (new_data_offset + size) < storage->data.size) {
		meta->entry.offset = new_data_offset;
		meta->entry.len = (uint32_t)size;
		return 0;
	}

	return -ENOSPC;
}

#if defined(FLASH_OVERWRITE)
static int delete_meta(struct storage *storage, struct meta *meta)
{
	struct meta_entry *entry = &meta->entry;

	entry->hash_murmur = 0;
	entry->hash_dbj2 = 0;

	return flash_write(storage->flash,
			meta->offset, entry, sizeof(*entry));
}
#endif

static int write_meta(struct storage *storage, const struct meta *meta)
{
	return flash_write(storage->flash,
			meta->offset, &meta->entry, sizeof(meta->entry));
}

static int write_value(struct storage *storage,
		const void *data, const struct meta *meta)
{
	uintptr_t offset = storage->offset +
		storage->meta.offset + storage->meta.size +
		meta->entry.offset;
	return flash_write(storage->flash, offset, data, meta->entry.len);
}

/* It should keep at least one validate partition even if an error occurs in the
 * middle of flash processing. */
static int reclaim(struct kvstore *self) {
        struct storage *p = &self->storage;
	struct storage *t = &self->scratch;

	if (!t->flash) {
		return -ENOTSUP;
	}

	uintptr_t start = p->offset + p->meta.offset;
	uintptr_t end = start + p->meta.size;
	uint32_t entry_size = sizeof(struct meta_entry);
	struct meta meta;

	flash_erase(t->flash, t->offset, t->size);

	for (uintptr_t offset = start; offset < end; offset += entry_size) {
		struct meta new_meta;

		if (flash_read(p->flash, offset, &meta.entry, entry_size) < 0) {
			return -EIO;
		}

		if (is_free_entry(&meta.entry) ||
				find_meta(p, &meta) != 0 ||
				find_meta(t, &meta) == 0) {
			continue;
		}

		if (alloc_entry(t, meta.entry.len, &new_meta) < 0) {
			return -EIO;
		}

		new_meta.entry.hash_murmur = meta.entry.hash_murmur;
		new_meta.entry.hash_dbj2 = meta.entry.hash_dbj2;

		if (write_meta(t, &new_meta) < 0) {
			return -EIO;
		}

		for (uint32_t i = 0; i < new_meta.entry.len; i++) {
			uint8_t buf[FLASH_LINE_ALIGN_BYTES];
                        uintptr_t ofs_f = p->offset + p->meta.offset +
				p->meta.size + meta.entry.offset + i;
                        uintptr_t ofs_t = t->offset + t->meta.offset +
				t->meta.size + new_meta.entry.offset + i;
                        if (flash_read(p->flash, ofs_f, buf, sizeof(buf)) < 0) {
                                return -EIO;
                        }
                        if (flash_write(t->flash, ofs_t, buf, sizeof(buf)) < 0) {
				return -EIO;
			}
		}
	}

	if (alloc_entry(t, FLASH_LINE_ALIGN_BYTES, &meta) == -ENOSPC) {
		return -ENOSPC;
	}

	flash_erase(p->flash, p->offset, p->size);

	for (uintptr_t offset = t->offset; offset < t->size;
			offset += FLASH_LINE_ALIGN_BYTES) {
		uint8_t buf[FLASH_LINE_ALIGN_BYTES];
		if (flash_read(t->flash, offset, buf, sizeof(buf)) < 0) {
			return -EIO;
		}
		if (flash_write(p->flash, offset, buf, sizeof(buf)) < 0) {
			return -EIO;
		}
	}

	return 0;
}

static int flash_kvstore_write(struct kvstore *self,
		const char *key, const void *value, size_t size)
{
	struct meta new_meta;
	int rc;

	if ((rc = alloc_entry(&self->storage, size, &new_meta)) != 0) {
		if (rc == -ENOSPC && (rc = reclaim(self)) == 0) {
			rc = alloc_entry(&self->storage, size, &new_meta);
		}

		if (rc != 0) {
			return rc;
		}
	}

#if defined(FLASH_OVERWRITE)
	struct meta old_meta;
	if (find_key(&self->storage, key, &old_meta) == 0) {
		delete_meta(&self->storage, &old_meta);
	}
#endif

	new_meta.entry.hash_murmur = hash_murmur_32(key);
	new_meta.entry.hash_dbj2 = hash_dbj2_32(key);

	if (write_meta(&self->storage, &new_meta) != 0 ||
			write_value(&self->storage, value, &new_meta) != 0) {
		return -EIO;
	}

	return 0;
}

static int flash_kvstore_read(struct kvstore *self,
		const char *key, void *buf, size_t size)
{
	struct meta meta;

	if (find_key(&self->storage, key, &meta) != 0) {
		return -ENOENT;
	}

	size_t len = MIN(size, meta.entry.len);
	uintptr_t offset = self->storage.offset +
		self->storage.meta.offset + self->storage.meta.size +
		meta.entry.offset;
	return flash_read(self->storage.flash, offset, buf, len);
}

static int flash_kvstore_erase(struct kvstore *self, const char *key)
{
#if defined(FLASH_OVERWRITE)
	struct meta meta;

	if (find_key(&self->storage, key, &meta) == 0) {
		return delete_meta(&self->storage, &meta);
	}

	return -ENOENT;
#else
	unused(self);
	unused(key);

	return -ENOTSUP;
#endif
}

static int flash_kvstore_open(struct kvstore *self, const char *namespace)
{
	/* WARN: only one namespace supported */
	unused(self);
	unused(namespace);
	return -EALREADY;
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
	nvs.storage.size = flash_size(flash);
	nvs.storage.offset = 0;

	/* meta takes about 6% of the given space */
	nvs.storage.meta.size = nvs.storage.size >> 4;
	nvs.storage.data.size = nvs.storage.size - nvs.storage.meta.size;
	nvs.storage.meta.offset = 0;
	nvs.storage.data.offset = 0;

	nvs.scratch.flash = scratch;
	nvs.scratch.offset = 0;
	if (scratch) {
		nvs.scratch.size = flash_size(scratch);

		nvs.scratch.meta.size = nvs.scratch.size >> 4;
		nvs.scratch.data.size =
			nvs.scratch.size - nvs.scratch.meta.size;
		nvs.scratch.meta.offset = 0;
		nvs.scratch.data.offset = 0;
	}

	return &nvs;
}
