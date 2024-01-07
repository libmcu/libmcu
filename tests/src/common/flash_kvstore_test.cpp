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

static size_t fake_size(struct flash *self)
{
	return (size_t)mock().actualCall(__func__)
		.withParameter("self", self)
		.returnUnsignedIntValueOrDefault(0);
}

static int fake_erase(struct flash *self, uintptr_t offset, size_t size)
{
	return mock().actualCall(__func__)
		.withParameter("self", self)
		.withParameter("offset", offset)
		.withParameter("size", size)
		.returnIntValueOrDefault(0);
}

static int fake_write(struct flash *self,
		uintptr_t offset, const void *data, size_t len)
{
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
		uintptr_t offset, void *buf, size_t len)
{
	return mock().actualCall(__func__)
		.withParameter("self", self)
		.withParameter("offset", offset)
		.withOutputParameter("buf", buf)
		.withParameter("len", len)
		.returnIntValueOrDefault(0);
}

static const uint32_t empty_line[] =
		{ 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };

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
		prepare_meta_read_any(0, META_SIZE / META_ENTRY_SIZE); // for find

		mock().expectNCalls(2, "fake_write") // for write meta and value
			.ignoreOtherParameters()
			.andReturnValue(0);

		kvstore_write(kvstore, key, value, value_size);
	}
};

TEST(FlashKVStore, open_ShouldAlwaysReturnSuccess) {
	LONGS_EQUAL(0, kvstore_open(kvstore, NULL));
	LONGS_EQUAL(0, kvstore_open(kvstore, "namespace"));
}

TEST(FlashKVStore, write_ShouldSucceed) {
	prepare_meta_read_empty(0, META_SIZE / META_ENTRY_SIZE); // for alloc
	prepare_meta_read_any(0, META_SIZE / META_ENTRY_SIZE); // for find

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

TEST(FlashKVStore, write_ShouldReturnNoSpace_WhenOutOfMemory) {
}

TEST(FlashKVStore, write_ShouldReclaim_WhenNoSpaceDueToFragmentation) {
}

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

	mock().expectOneCall("fake_read") // for find
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
	prepare_meta_read_any(0, META_SIZE / META_ENTRY_SIZE);
	LONGS_EQUAL(-ENOENT, kvstore_clear(kvstore, "nonexistkey"));
}

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
