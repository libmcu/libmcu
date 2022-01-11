#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"

#include "memory_kvstore.h"

TEST_GROUP(MemoryKVStore) {
	struct kvstore *storage;

	void setup(void) {
		memory_kvstore_init();
		storage = memory_kvstore_create("namespace");
	}
	void teardown() {
		memory_kvstore_destroy(storage);
	}
};

TEST(MemoryKVStore, write_ShouldReturnSizeOfWritten) {
	int8_t v8;
	int16_t v16;
	int32_t v32;
	int64_t v64;
	char s[80];

	LONGS_EQUAL(sizeof(v8), kvstore_write(storage, "v8", &v8, sizeof(v8)));
	LONGS_EQUAL(sizeof(v16), kvstore_write(storage, "v16", &v16, sizeof(v16)));
	LONGS_EQUAL(sizeof(v32), kvstore_write(storage, "v32", &v32, sizeof(v32)));
	LONGS_EQUAL(sizeof(v64), kvstore_write(storage, "v64", &v64, sizeof(v64)));
	LONGS_EQUAL(sizeof(s), kvstore_write(storage, "str", s, sizeof(s)));
}

TEST(MemoryKVStore, write_ShouldOverwrite_WhenTheSameKeyProvided) {
	uint32_t val_written = 0xdeadbeef;
	uint32_t val_overwritten = 0xbaadf00d;
	uint32_t val_read = 0;
	kvstore_write(storage, "key", &val_written, sizeof(val_written));
	kvstore_write(storage, "key", &val_overwritten, sizeof(val_overwritten));
	kvstore_read(storage, "key", &val_read, sizeof(val_read));
	LONGS_EQUAL(val_overwritten, val_read);
}

TEST(MemoryKVStore, write_ShouldWrite) {
	uint32_t val1 = 0xc0defeed;
	uint32_t val2 = 0xc1defeed;
	uint32_t val3 = 0xc2defeed;
	uint32_t val4 = 0xc3defeed;
	uint32_t val_read = 0;
	kvstore_write(storage, "key1", &val1, sizeof(val1));
	kvstore_write(storage, "key2", &val2, sizeof(val2));
	kvstore_write(storage, "key3", &val3, sizeof(val3));
	kvstore_write(storage, "key4", &val4, sizeof(val4));
	kvstore_read(storage, "key1", &val_read, sizeof(val_read));
	LONGS_EQUAL(val1, val_read);
	kvstore_read(storage, "key2", &val_read, sizeof(val_read));
	LONGS_EQUAL(val2, val_read);
	kvstore_read(storage, "key3", &val_read, sizeof(val_read));
	LONGS_EQUAL(val3, val_read);
	kvstore_read(storage, "key4", &val_read, sizeof(val_read));
	LONGS_EQUAL(val4, val_read);
}

TEST(MemoryKVStore, write_ShouldReturnZero_WhenAllocFails) {
	uint32_t val1 = 0xc0defeed;
	cpputest_malloc_set_out_of_memory_countdown(1);
	LONGS_EQUAL(0, kvstore_write(storage, "key1", &val1, sizeof(val1)));
	cpputest_malloc_set_not_out_of_memory();

	cpputest_malloc_set_out_of_memory_countdown(2);
	LONGS_EQUAL(0, kvstore_write(storage, "key1", &val1, sizeof(val1)));
	cpputest_malloc_set_not_out_of_memory();

	cpputest_malloc_set_out_of_memory_countdown(3);
	LONGS_EQUAL(0, kvstore_write(storage, "key1", &val1, sizeof(val1)));
	cpputest_malloc_set_not_out_of_memory();

	cpputest_malloc_set_out_of_memory_countdown(4);
	LONGS_EQUAL(sizeof(val1), kvstore_write(storage, "key1", &val1, sizeof(val1)));
	cpputest_malloc_set_not_out_of_memory();

	cpputest_malloc_set_out_of_memory();
	LONGS_EQUAL(0, kvstore_write(storage, "key1", &val1, sizeof(val1)));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(MemoryKVStore, read_ShouldReturnTheExactValueWritten) {
	uint32_t val_written = 0xc0decafe;
	uint32_t val_read = 0;
	kvstore_write(storage, "key", &val_written, sizeof(val_written));
	kvstore_read(storage, "key", &val_read, sizeof(val_read));
	LONGS_EQUAL(val_written, val_read);
}

TEST(MemoryKVStore, read_ShouldReturnZero_WhenNoEntryForTheKey) {
	uint8_t val_read[80];
	LONGS_EQUAL(0, kvstore_read(storage, "key", &val_read, sizeof(val_read)));
}

TEST(MemoryKVStore, new_ShouldReturnExistNamespace_WhenAlreadyCreated) {
	uint32_t val_written = 0xdeadbeef;
	uint32_t val_read = 0;
	kvstore_write(storage, "key", &val_written, sizeof(val_written));
	struct kvstore *same_ns = memory_kvstore_create("namespace");
	kvstore_read(same_ns, "key", &val_read, sizeof(val_read));
	LONGS_EQUAL(val_written, val_read);
}

TEST(MemoryKVStore, new_ShouldKeepDataSeperatedFromOtherNamespaces_WhenNewNamespaceCreated) {
	uint32_t val1_written = 0xdeafc0de;
	uint32_t val1_read = 0;
	uint32_t val2_written = 0xdeafc1de;
	uint32_t val2_read = 0;

	struct kvstore *ns2 = memory_kvstore_create("another namespace");

	kvstore_write(storage, "samekey", &val1_written, sizeof(val1_written));
	kvstore_write(ns2, "samekey", &val2_written, sizeof(val2_written));

	kvstore_read(storage, "samekey", &val1_read, sizeof(val1_read));
	LONGS_EQUAL(val1_written, val1_read);
	kvstore_read(ns2, "samekey", &val2_read, sizeof(val2_read));
	LONGS_EQUAL(val2_written, val2_read);

	memory_kvstore_destroy(ns2);
}

TEST(MemoryKVStore, new_ShouldReturnNull_WhenAllocFails) {
	cpputest_malloc_set_out_of_memory();
	POINTERS_EQUAL(NULL, memory_kvstore_create("newNs"));
	cpputest_malloc_set_not_out_of_memory();

	cpputest_malloc_set_out_of_memory_countdown(2);
	POINTERS_EQUAL(NULL, memory_kvstore_create("newNs"));
	cpputest_malloc_set_not_out_of_memory();
}
