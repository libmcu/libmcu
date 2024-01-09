/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"
#include "bytearray.h"

#include "libmcu/flash_kvstore.h"

#define STORAGE_SIZE		1024
#define META_SIZE		(1024 >> 4)
#define META_ENTRY_SIZE		16

struct flash {
	struct flash_api api;
};

static uint8_t spy_flash[STORAGE_SIZE];
static uint8_t spy_flash_scratch[STORAGE_SIZE];

static size_t spy_size(struct flash *self) {
	return sizeof(spy_flash);
}
static int spy_erase(struct flash *self, uintptr_t offset, size_t size) {
	memset(&spy_flash[offset], 0xff, size);
	return 0;
}
static int spy_write(struct flash *self,
		uintptr_t offset, const void *data, size_t len) {
	memcpy(&spy_flash[offset], data, len);
	return 0;
}
static int spy_read(struct flash *self,
		uintptr_t offset, void *buf, size_t len) {
	memcpy(buf, &spy_flash[offset], len);
	return (int)len;
}

static size_t spy_scratch_size(struct flash *self) {
	return sizeof(spy_flash_scratch);
}
static int spy_scratch_erase(struct flash *self, uintptr_t offset, size_t size) {
	memset(&spy_flash_scratch[offset], 0xff, size);
	return 0;
}
static int spy_scratch_write(struct flash *self,
		uintptr_t offset, const void *data, size_t len) {
	memcpy(&spy_flash_scratch[offset], data, len);
	return 0;
}
static int spy_scratch_read(struct flash *self,
		uintptr_t offset, void *buf, size_t len) {
	memcpy(buf, &spy_flash_scratch[offset], len);
	return (int)len;
}

static size_t fake_size(struct flash *self) {
	return (size_t)mock().actualCall(__func__)
		.withParameter("self", self)
		.returnUnsignedIntValueOrDefault(0);
}
static int fake_erase(struct flash *self, uintptr_t offset, size_t size) {
	return mock().actualCall(__func__)
		.withParameter("self", self)
		.withParameter("offset", offset)
		.withParameter("size", size)
		.returnIntValueOrDefault(0);
}
static int fake_write(struct flash *self,
		uintptr_t offset, const void *data, size_t len) {
	ByteArray byteArray = {
		data, len,
	};
	return mock().actualCall(__func__)
		.withParameter("self", self)
		.withParameter("offset", offset)
		.withParameterOfType("ByteArray", "byteArray", &byteArray)
		.returnIntValueOrDefault(0);
}
static int fake_read(struct flash *self,
		uintptr_t offset, void *buf, size_t len) {
	return mock().actualCall(__func__)
		.withParameter("self", self)
		.withParameter("offset", offset)
		.withOutputParameter("buf", buf)
		.withParameter("len", len)
		.returnIntValueOrDefault(0);
}

TEST_GROUP(FlashKVStore) {
	ByteArrayComparator comparator;

	struct kvstore *kvstore;
	struct flash flash;

	void setup(void) {
		mock().installComparator("ByteArray", comparator);

		flash.api.erase = fake_erase;
		flash.api.write = fake_write;
		flash.api.read = fake_read;
		flash.api.size = fake_size;

		memset(spy_flash, 0xff, sizeof(spy_flash));
		memset(spy_flash_scratch, 0xff, sizeof(spy_flash_scratch));

		mock().expectOneCall("fake_size")
			.ignoreOtherParameters()
			.andReturnValue(STORAGE_SIZE);
		kvstore = flash_kvstore_new(&flash, NULL);
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().removeAllComparatorsAndCopiers();
		mock().clear();
	}

	void prepare_meta_read_empty(int start, int n) {
		static const uint32_t empty_line[] =
			{ 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };

		for (int i = start; i < start + n; i++) {
			mock().expectOneCall("fake_read")
				.withParameter("self", &flash)
				.withParameter("offset", i * META_ENTRY_SIZE)
				.withParameter("len", META_ENTRY_SIZE)
				.withOutputParameterReturning("buf",
						empty_line, sizeof(empty_line))
				.andReturnValue(0);
		}
	}
	void prepare_meta_read_any(int start, int n) {
		for (int i = start; i < start + n; i++) {
			mock().expectOneCall("fake_read")
				.withParameter("self", &flash)
				.withParameter("offset", i * META_ENTRY_SIZE)
				.withParameter("len", META_ENTRY_SIZE)
				.ignoreOtherParameters()
				.andReturnValue(0);
		}
	}
	void write_key_value(const char *key, const void *value, size_t value_size) {
		prepare_meta_read_empty(0, META_SIZE / META_ENTRY_SIZE); // for alloc
#if defined(FLASH_OVERWRITE)
		prepare_meta_read_any(0, META_SIZE / META_ENTRY_SIZE); // for find
#endif
		mock().expectNCalls(2, "fake_write") // for write meta and value
			.ignoreOtherParameters()
			.andReturnValue(0);

		kvstore_write(kvstore, key, value, value_size);
	}
};

TEST(FlashKVStore, open_ShouldAlwaysReturnAlready) {
	LONGS_EQUAL(-EALREADY, kvstore_open(kvstore, NULL));
	LONGS_EQUAL(-EALREADY, kvstore_open(kvstore, "namespace"));
}

TEST(FlashKVStore, write_ShouldSucceed) {
	prepare_meta_read_empty(0, META_SIZE / META_ENTRY_SIZE); // for alloc
#if defined(FLASH_OVERWRITE)
	prepare_meta_read_any(0, META_SIZE / META_ENTRY_SIZE); // for find
#endif

	const uint8_t expected_value[] = {
		'm', 'y', 'v', 'a', 'l', 'u', 'e',
	};
	const uint8_t expected_meta[] = {
		0x34, 0xfd, 0x64, 0x71, 0x94, 0x9d, 0xf7, 0x0f,
		0x00, 0x00, 0x00, 0x00, sizeof(expected_value), 0x00, 0x00, 0x00,
	};
	ByteArray meta = { expected_meta, sizeof(expected_meta), };
	ByteArray value = { expected_value, sizeof(expected_value), };
	mock().expectOneCall("fake_write") // for write meta
		.withParameter("self", &flash)
		.withParameter("offset", 0)
		.withParameterOfType("ByteArray", "byteArray", &meta)
		.andReturnValue(0);
	mock().expectOneCall("fake_write") // for write value
		.withParameter("self", &flash)
		.withParameter("offset", 64)
		.withParameterOfType("ByteArray", "byteArray", &value)
		.andReturnValue(0);

	LONGS_EQUAL(0, kvstore_write(kvstore, "mykey",
			expected_value, sizeof(expected_value)));
}

#if defined(FLASH_OVERWRITE)
TEST(FlashKVStore, write_ShouldDeleteFirst_WhenTheSameEntryExist) {
	const uint8_t new_value[] = {
		'n', 'e', 'w', ' ', 'v', 'a', 'l', 'u', 'e',
	};
	const uint8_t exist_value[] = {
		'm', 'y', 'v', 'a', 'l', 'u', 'e',
	};
	const uint8_t exist_meta[] = {
		0x34, 0xfd, 0x64, 0x71, 0x94, 0x9d, 0xf7, 0x0f,
		0x00, 0x00, 0x00, 0x00, sizeof(exist_value), 0x00, 0x00, 0x00,
	};
	const uint8_t deleted_meta[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, sizeof(exist_value), 0x00, 0x00, 0x00,
	};
	const uint8_t new_meta[] = {
		0x34, 0xfd, 0x64, 0x71, 0x94, 0x9d, 0xf7, 0x0f,
		sizeof(exist_value), 0x00, 0x00, 0x00, sizeof(new_value), 0x00, 0x00, 0x00,
	};
	ByteArray value = { new_value, sizeof(new_value), };
	ByteArray meta = { new_meta, sizeof(new_meta), };
	ByteArray meta_deleted = { deleted_meta, sizeof(deleted_meta), };

	mock().expectOneCall("fake_read")
		.withParameter("self", &flash)
		.withParameter("offset", 0)
		.withParameter("len", META_ENTRY_SIZE)
		.withOutputParameterReturning("buf",
				exist_meta, sizeof(exist_meta))
		.andReturnValue(0);
	prepare_meta_read_empty(1, META_SIZE / META_ENTRY_SIZE - 1); // for alloc

	mock().expectOneCall("fake_read") // for find
		.withParameter("self", &flash)
		.withParameter("offset", 0)
		.withParameter("len", META_ENTRY_SIZE)
		.withOutputParameterReturning("buf",
				exist_meta, sizeof(exist_meta))
		.andReturnValue(0);

	mock().expectOneCall("fake_write") // for delete meta
		.withParameter("self", &flash)
		.withParameter("offset", 0)
		.withParameterOfType("ByteArray", "byteArray", &meta_deleted)
		.andReturnValue(0);
	mock().expectOneCall("fake_write") // for write meta
		.withParameter("self", &flash)
		.withParameter("offset", 16)
		.withParameterOfType("ByteArray", "byteArray", &meta)
		.andReturnValue(0);
	mock().expectOneCall("fake_write") // for write value
		.withParameter("self", &flash)
		.withParameter("offset", 64+7)
		.withParameterOfType("ByteArray", "byteArray", &value)
		.andReturnValue(0);

	LONGS_EQUAL(0, kvstore_write(kvstore,
			"mykey", new_value, sizeof(new_value)));
}
#endif

TEST(FlashKVStore, read_ShouldReturnNoEntry_WhenGivenKeyNotFound) {
	uint8_t buf[64];
	prepare_meta_read_any(0, META_SIZE / META_ENTRY_SIZE);
	LONGS_EQUAL(-ENOENT, kvstore_read(kvstore, "nonexistkey", buf, sizeof(buf)));
}

TEST(FlashKVStore, read_ShouldReturnLengthOfBytesRead) {
	const uint8_t data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	const char *key = "key string";
	write_key_value(key, data, sizeof(data));

	uint8_t buf[64];
	const uint8_t exist_meta[] = {
		0xab, 0x13, 0x35, 0x24, 0x25, 0xff, 0x4a, 0x3a,
		0x00, 0x00, 0x00, 0x00, sizeof(data), 0x00, 0x00, 0x00,
	};
	mock().expectOneCall("fake_read") // for find
		.withParameter("self", &flash)
		.withParameter("offset", 0)
		.withParameter("len", META_ENTRY_SIZE)
		.withOutputParameterReturning("buf",
				exist_meta, sizeof(exist_meta))
		.andReturnValue(0);
	prepare_meta_read_empty(1, META_SIZE / META_ENTRY_SIZE - 1);

	mock().expectOneCall("fake_read") // for value read
		.withParameter("self", &flash)
		.withParameter("offset", 64)
		.withParameter("len", sizeof(data))
		.withOutputParameterReturning("buf",
				data, sizeof(data))
		.andReturnValue((int)sizeof(data));
	LONGS_EQUAL(sizeof(data), kvstore_read(kvstore, key, buf, sizeof(buf)));
	MEMCMP_EQUAL(data, buf, sizeof(data));
}

TEST(FlashKVStore, erase_ShouldReturnNoEntry_WhenNonExistKeyGiven) {
#if defined(FLASH_OVERWRITE)
	prepare_meta_read_any(0, META_SIZE / META_ENTRY_SIZE);
	LONGS_EQUAL(-ENOENT, kvstore_clear(kvstore, "nonexistkey"));
#else
	LONGS_EQUAL(-ENOTSUP, kvstore_clear(kvstore, "nonexistkey"));
#endif
}

#if defined(FLASH_OVERWRITE)
TEST(FlashKVStore, erase_ShouldDeleteMetaEntry_WhenMatchingKeyGiven) {
	const uint8_t data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	const char *key = "key string";
	write_key_value(key, data, sizeof(data));

	const uint8_t exist_meta[] = {
		0xab, 0x13, 0x35, 0x24, 0x25, 0xff, 0x4a, 0x3a,
		0x00, 0x00, 0x00, 0x00, sizeof(data), 0x00, 0x00, 0x00,
	};
	const uint8_t deleted_meta[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, sizeof(data), 0x00, 0x00, 0x00,
	};
	ByteArray meta_deleted = { deleted_meta, sizeof(deleted_meta), };

	mock().expectOneCall("fake_read") // for find
		.withParameter("self", &flash)
		.withParameter("offset", 0)
		.withParameter("len", META_ENTRY_SIZE)
		.withOutputParameterReturning("buf",
				exist_meta, sizeof(exist_meta))
		.andReturnValue(0);
	mock().expectOneCall("fake_write") // for delete meta
		.withParameter("self", &flash)
		.withParameter("offset", 0)
		.withParameterOfType("ByteArray", "byteArray", &meta_deleted)
		.andReturnValue(0);

	LONGS_EQUAL(0, kvstore_clear(kvstore, key));
}
#endif

TEST(FlashKVStore, read_ShouldReturnLatestEntry_WhenTheSameKeyInMultipleEntriesGiven) {
	uint8_t buf[64];
	const uint8_t first_data[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	const uint8_t second_data[] = { 0xa, 0xb, 0xc };
	const uint8_t first_meta[] = {
		0xab, 0x13, 0x35, 0x24, 0x25, 0xff, 0x4a, 0x3a,
		0x00, 0x00, 0x00, 0x00, sizeof(first_data), 0x00, 0x00, 0x00,
	};
	const uint8_t second_meta[] = {
		0xab, 0x13, 0x35, 0x24, 0x25, 0xff, 0x4a, 0x3a,
		META_ENTRY_SIZE, 0x00, 0x00, 0x00, sizeof(second_data), 0x00, 0x00, 0x00,
	};
	const char *key = "key string";

	mock().expectOneCall("fake_read") // for find
		.withParameter("self", &flash)
		.withParameter("offset", 0)
		.withParameter("len", sizeof(first_meta))
		.withOutputParameterReturning("buf",
				first_meta, sizeof(first_meta))
		.andReturnValue((int)sizeof(first_meta));
	mock().expectOneCall("fake_read")
		.withParameter("self", &flash)
		.withParameter("offset", META_ENTRY_SIZE)
		.withParameter("len", sizeof(second_meta))
		.withOutputParameterReturning("buf",
				second_meta, sizeof(second_meta))
		.andReturnValue((int)sizeof(second_meta));
	prepare_meta_read_empty(2, META_SIZE / META_ENTRY_SIZE - 2);

	mock().expectOneCall("fake_read") // for value read
		.withParameter("self", &flash)
		.withParameter("offset", 64 + META_ENTRY_SIZE)
		.withParameter("len", sizeof(second_data))
		.withOutputParameterReturning("buf",
				second_data, sizeof(second_data))
		.andReturnValue((int)sizeof(second_data));

	LONGS_EQUAL(sizeof(second_data), kvstore_read(kvstore, key, buf, sizeof(buf)));
	MEMCMP_EQUAL(second_data, buf, sizeof(second_data));
}

TEST(FlashKVStore, write_ShouldWriteInAlignment) {
	const uint8_t first_meta[] = {
		0xab, 0x13, 0x35, 0x24, 0x25, 0xff, 0x4a, 0x3a,
		0x00, 0x00, 0x00, 0x00, 9, 0x00, 0x00, 0x00,
	};

	mock().expectOneCall("fake_read") // for find
		.withParameter("self", &flash)
		.withParameter("offset", 0)
		.withParameter("len", sizeof(first_meta))
		.withOutputParameterReturning("buf",
				first_meta, sizeof(first_meta))
		.andReturnValue((int)sizeof(first_meta));
	prepare_meta_read_empty(1, META_SIZE / META_ENTRY_SIZE - 1);

	const uint8_t expected_value[] = { 1, 2, 3 };
	const uint8_t expected_meta[] = {
		0x34, 0xfd, 0x64, 0x71, 0x94, 0x9d, 0xf7, 0x0f,
		META_ENTRY_SIZE, 0x00, 0x00, 0x00, sizeof(expected_value), 0x00, 0x00, 0x00,
	};
	ByteArray meta = { expected_meta, sizeof(expected_meta), };
	ByteArray value = { expected_value, sizeof(expected_value), };
	mock().expectOneCall("fake_write") // for write meta
		.withParameter("self", &flash)
		.withParameter("offset", META_ENTRY_SIZE)
		.withParameterOfType("ByteArray", "byteArray", &meta)
		.andReturnValue(0);
	mock().expectOneCall("fake_write") // for write value
		.withParameter("self", &flash)
		.withParameter("offset", 64 + META_ENTRY_SIZE)
		.withParameterOfType("ByteArray", "byteArray", &value)
		.andReturnValue(0);

	LONGS_EQUAL(0, kvstore_write(kvstore, "mykey",
			expected_value, sizeof(expected_value)));
}

TEST(FlashKVStore, write_ShouldReturnNoSpace_WhenNoSpaceLeft) {
	struct flash f = {
		.api = {
			.erase = spy_erase,
			.write = spy_write,
			.read = spy_read,
			.size = spy_size,
		},
	};
	struct flash s = {
		.api = {
			.erase = spy_scratch_erase,
			.write = spy_scratch_write,
			.read = spy_scratch_read,
			.size = spy_scratch_size,
		},
	};
	kvstore = flash_kvstore_new(&f, &s);
	const char *keys[] = {
		"key1",
		"key2",
		"key3",
		"key4",
		"key5",
	};
	const uint8_t data[] = { 0xde, 0xad, 0xc0, 0xde };
	for (int i = 0; i < 4; i++) {
		LONGS_EQUAL(0, kvstore_write(kvstore, keys[i], data, sizeof(data)));
	}
	LONGS_EQUAL(-ENOSPC, kvstore_write(kvstore, keys[4], data, sizeof(data)));
}

TEST(FlashKVStore, write_ShouldReclaim_WhenNoSpaceDueToFragmentation) {
	struct flash f = {
		.api = {
			.erase = spy_erase,
			.write = spy_write,
			.read = spy_read,
			.size = spy_size,
		},
	};
	struct flash s = {
		.api = {
			.erase = spy_scratch_erase,
			.write = spy_scratch_write,
			.read = spy_scratch_read,
			.size = spy_scratch_size,
		},
	};
	kvstore = flash_kvstore_new(&f, &s);
	const char *keys[] = {
		"key1",
		"key1",
		"key3",
		"key4",
		"key4",
		"key5",
	};
	const uint8_t data[][1] = { 1, 2, 3, 4, 5, 6 };
	for (int i = 0; i < 5; i++) {
		LONGS_EQUAL(0, kvstore_write(kvstore, keys[i], data[i], sizeof(data[i])));
	}
	LONGS_EQUAL(0, kvstore_write(kvstore, keys[5], data[5], sizeof(data[5])));
}
