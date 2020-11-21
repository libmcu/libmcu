#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <string.h>
#include "libmcu/nvs_kvstore.h"

TEST_GROUP(NVSKVStore) {
	void setup(void) {
		mock().ignoreOtherCalls();
	}
	void teardown() {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(NVSKVStore, init_ShouldCall_nvs_flash_init) {
	mock().expectOneCall("nvs_flash_init").andReturnValue(0);
	LONGS_EQUAL(0, nvs_kvstore_init());
}

TEST(NVSKVStore, new_ShouldCall_nvs_open) {
	mock().expectOneCall("nvs_open").andReturnValue(-1);
	LONGS_EQUAL(0, nvs_kvstore_new("namespace"));
}

TEST(NVSKVStore, new_ShouldReturnNull_When_nvs_open_fails) {
	mock().expectOneCall("nvs_open").andReturnValue(-1);
	LONGS_EQUAL(0, nvs_kvstore_new("namespace"));
}

TEST(NVSKVStore, new_ShouldReturnKVStoreObject) {
	mock().expectOneCall("nvs_open").andReturnValue(0);
	kvstore_t *kvstore = nvs_kvstore_new("namespace");
	CHECK(kvstore != NULL);
	nvs_kvstore_delete(kvstore);
}

TEST(NVSKVStore, write_ShouldCall_nvs_set_blob) {
	kvstore_t *kvstore = nvs_kvstore_new("namespace");
	mock().expectOneCall("nvs_set_blob").andReturnValue(1);
	kvstore->write(kvstore, "key", "value", 5);
	nvs_kvstore_delete(kvstore);
}

TEST(NVSKVStore, write_ShouldCall_nvs_commit) {
	kvstore_t *kvstore = nvs_kvstore_new("namespace");
	mock().expectOneCall("nvs_commit").andReturnValue(1);
	kvstore_write(kvstore, "key", "value", 5);
	nvs_kvstore_delete(kvstore);
}

IGNORE_TEST(NVSKVStore, write_ShouldReturnWrittenBytes) {
	kvstore_t *kvstore = nvs_kvstore_new("namespace");
	LONGS_EQUAL(5, kvstore_write(kvstore, "key", "value", 5));
	nvs_kvstore_delete(kvstore);
}

TEST(NVSKVStore, read_ShouldCall_nvs_get_blob) {
	char buf[10];
	kvstore_t *kvstore = nvs_kvstore_new("namespace");
	mock().expectOneCall("nvs_get_blob").andReturnValue(1);
	kvstore_read(kvstore, "key", buf, 10);
	nvs_kvstore_delete(kvstore);
}

IGNORE_TEST(NVSKVStore, read_ShouldReturnReadBytes) {
	char buf[10];
	kvstore_t *kvstore = nvs_kvstore_new("namespace");
	LONGS_EQUAL(10, kvstore_read(kvstore, "key", buf, 10));
	nvs_kvstore_delete(kvstore);
}
