/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "libmcu/metricfs.h"

#define MAX_METRICS		3

struct kvstore {
	kvstore_api api;
};

static int fake_write(struct kvstore *self,
		const char *key, const void *value, size_t size) {
	return mock().actualCall("fake_write")
		.withParameter("key", key)
		.withMemoryBufferParameter("value", (const uint8_t *)value, size)
		.returnIntValue();
}
static int fake_read(struct kvstore *self,
		const char *key, void *buf, size_t size) {
	return mock().actualCall("fake_read")
		.withParameter("key", key)
		.withOutputParameter("buf", buf)
		.returnIntValue();
}
static int fake_clear(struct kvstore *self, const char *key) {
	return mock().actualCall("fake_clear")
		.withParameter("key", key)
		.returnIntValue();
}

static struct kvstore fake_kvstore = {
	.api = {
		.write = fake_write,
		.read = fake_read,
		.clear = fake_clear,
	},
};

static void on_iterate(const metricfs_id_t id, const void *data, const size_t datasize, void *ctx) {
	mock().actualCall("on_iterate").withParameter("id", (metricfs_id_t)id);
}

TEST_GROUP(metricfs) {
	struct metricfs *fs;
	uint16_t index;
	uint16_t outdex;

	void setup(void) {
		index = 0;
		outdex = 0;

		mock().expectOneCall("fake_read")
			.withParameter("key", "prefix/index")
			.withOutputParameterReturning("buf", &index, sizeof(index))
			.andReturnValue((int)sizeof(index));
		mock().expectOneCall("fake_read")
			.withParameter("key", "prefix/outdex")
			.withOutputParameterReturning("buf", &outdex, sizeof(outdex))
			.andReturnValue((int)sizeof(outdex));
		fs = metricfs_create(&fake_kvstore, "prefix", MAX_METRICS);
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();

		metricfs_destroy(fs);
	}

	void expect_index_write(const char *idstr, const uint8_t *data,
			const size_t datasize) {
		index += 1;
		mock().expectOneCall("fake_write")
			.withParameter("key", idstr)
			.withMemoryBufferParameter("value", data, datasize)
			.andReturnValue((int)datasize);
		mock().expectOneCall("fake_write")
			.withParameter("key", "prefix/index")
			.withMemoryBufferParameter("value", (const uint8_t *)&index, sizeof(index))
			.andReturnValue((int)sizeof(index));
	}
	void expect_outdex_write(const char *idstr, const uint8_t *data,
			const size_t datasize) {
		outdex += 1;
		mock().expectOneCall("fake_clear")
			.withParameter("key", idstr)
			.andReturnValue(0);
		mock().expectOneCall("fake_write")
			.withParameter("key", "prefix/outdex")
			.withMemoryBufferParameter("value", (const uint8_t *)&outdex, sizeof(outdex))
			.andReturnValue((int)sizeof(outdex));
	}
	void expect_peek(const char *idstr, const uint8_t *data,
			const size_t datasize) {
		mock().expectOneCall("fake_read")
			.withParameter("key", idstr)
			.withOutputParameterReturning("buf", data, datasize)
			.andReturnValue((int)datasize);
	}
	void expect_read(const char *idstr, const uint8_t *data,
			const size_t datasize) {
		mock().expectOneCall("fake_read")
			.withParameter("key", idstr)
			.withOutputParameterReturning("buf", data, datasize)
			.andReturnValue((int)datasize);
		expect_outdex_write(idstr, data, datasize);
	}
};

TEST(metricfs, clear_ShouldReturnENOENT_WhenNoMetrics) {
	LONGS_EQUAL(-ENOENT, metricfs_clear(fs));
}
TEST(metricfs, del_first_ShouldReturnENOENT_WhenNoMetrics) {
	LONGS_EQUAL(-ENOENT, metricfs_del_first(fs, NULL));
}
TEST(metricfs, peek_first_ShouldReturnENOENT_WhenNoMetrics) {
	uint8_t buf[8];
	LONGS_EQUAL(-ENOENT, metricfs_peek_first(fs, buf, sizeof(buf), NULL));
}
TEST(metricfs, peek_ShouldReturnENOENT_WhenNoMatchingMetrics) {
	uint8_t buf[8];
	mock().expectOneCall("fake_read")
		.withParameter("key", "prefix/0")
		.ignoreOtherParameters()
		.andReturnValue(-ENOENT);
	LONGS_EQUAL(-ENOENT, metricfs_peek(fs, 0, buf, sizeof(buf)));
}
TEST(metricfs, count_ShouldReturnZero_WhenNoMetrics) {
	LONGS_EQUAL(0, metricfs_count(fs));
}
TEST(metricfs, iterate_ShouldReturnENOENT_WhenNoMetrics) {
	LONGS_EQUAL(-ENOENT, metricfs_iterate(fs, on_iterate, NULL, NULL, 0, 0));
}

TEST(metricfs, write_ShouldWriteInStorage_WhenCalled) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
}
TEST(metricfs, write_ShouldReturnWrttenId_WhenIdNotNull) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	metricfs_id_t id;
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), &id));
	LONGS_EQUAL(0, id);
}
TEST(metricfs, write_ShouldIncrementIndex_WhenCalled) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	LONGS_EQUAL(1, metricfs_count(fs));
}
TEST(metricfs, write_ShouldReturnENOSPC_WhenStorageFull) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};

	for (metricfs_id_t i = 0; i < MAX_METRICS; i++) {
		char idstr[32];
		snprintf(idstr, sizeof(idstr)-1, "prefix/%u", i);
		expect_index_write(idstr, data, sizeof(data));
		LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	}
	LONGS_EQUAL(-ENOSPC, metricfs_write(fs, data, sizeof(data), NULL));
}

TEST(metricfs, del_first_ShouldDeleteFirstMetric_WhenCalled) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	expect_index_write("prefix/1", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));

	metricfs_id_t id;
	expect_outdex_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_del_first(fs, &id));
	LONGS_EQUAL(0, id);
}

TEST(metricfs, peek_first_ShouldPeekFirstMetric_WhenCalled) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	expect_index_write("prefix/1", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));

	uint8_t buf[8];
	metricfs_id_t id;
	expect_peek("prefix/0", data, sizeof(data));
	LONGS_EQUAL(8, metricfs_peek_first(fs, buf, sizeof(buf), &id));
	MEMCMP_EQUAL(data, buf, sizeof(data));
	LONGS_EQUAL(0, id);
}

TEST(metricfs, read_first_ShouldReadFirstMetric_WhenCalled) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	expect_index_write("prefix/1", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));

	uint8_t buf[8];
	metricfs_id_t id;
	expect_read("prefix/0", data, sizeof(data));
	LONGS_EQUAL(8, metricfs_read_first(fs, buf, sizeof(buf), &id));
	MEMCMP_EQUAL(data, buf, sizeof(data));
	LONGS_EQUAL(0, id);
}

TEST(metricfs, ShouldIncreaseIndexAndOutdexMonotonicly_WhenWriteAndReadRepeatedly) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	uint8_t buf[8];
	metricfs_id_t id;

	for (metricfs_id_t i = 0; i < MAX_METRICS*2; i++) {
		char idstr[32];
		snprintf(idstr, sizeof(idstr)-1, "prefix/%u", i);

		expect_index_write(idstr, data, sizeof(data));
		LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
		expect_read(idstr, data, sizeof(data));
		LONGS_EQUAL(8, metricfs_read_first(fs, buf, sizeof(buf), &id));
		LONGS_EQUAL(i, id);
	}
}

TEST(metricfs, ShouldStillWork_WhenIndexWrapAround) {
	metricfs_destroy(fs);
	index = 3;
	outdex = 0xfffe;

	mock().expectOneCall("fake_read")
		.withParameter("key", "prefix/index")
		.withOutputParameterReturning("buf", &index, sizeof(index))
		.andReturnValue((int)sizeof(index));
	mock().expectOneCall("fake_read")
		.withParameter("key", "prefix/outdex")
		.withOutputParameterReturning("buf", &outdex, sizeof(outdex))
		.andReturnValue((int)sizeof(outdex));
	fs = metricfs_create(&fake_kvstore, "prefix", 10);

	LONGS_EQUAL(5, metricfs_count(fs));

	metricfs_id_t id;
	uint8_t buf[8];
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/3", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));

	expect_read("prefix/65534", data, sizeof(data));
	LONGS_EQUAL(8, metricfs_read_first(fs, buf, sizeof(buf), &id));
	LONGS_EQUAL(65534, id);
}

TEST(metricfs, ShouldReturnZeroIndex_WhenIndexIsNotCreatedYet) {
	metricfs_destroy(fs);
	mock().expectOneCall("fake_read")
		.withParameter("key", "prefix/index")
		.ignoreOtherParameters()
		.andReturnValue(-ENOENT);
	mock().expectOneCall("fake_read")
		.withParameter("key", "prefix/outdex")
		.ignoreOtherParameters()
		.andReturnValue(-ENOENT);
	fs = metricfs_create(&fake_kvstore, "prefix", 10);
	CHECK(fs != NULL);

	metricfs_id_t id;
	uint8_t buf[8];
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	expect_read("prefix/0", data, sizeof(data));
	LONGS_EQUAL(8, metricfs_read_first(fs, buf, sizeof(buf), &id));
	LONGS_EQUAL(0, id);
}

TEST(metricfs, write_ShouldNotIncreaseIndex_WhenWriteFailed) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	expect_index_write("prefix/1", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));

	mock().expectOneCall("fake_write")
		.withParameter("key", "prefix/2")
		.ignoreOtherParameters()
		.andReturnValue(-ENOMEM);
	LONGS_EQUAL(-ENOMEM, metricfs_write(fs, data, sizeof(data), NULL));
	LONGS_EQUAL(2, metricfs_count(fs));
}
TEST(metricfs, write_ShouldNotIncreaseIndex_WhenIndexWriteFailed) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	expect_index_write("prefix/1", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));

	mock().expectOneCall("fake_write")
		.withParameter("key", "prefix/2")
		.ignoreOtherParameters()
		.andReturnValue((int)sizeof(data));
	mock().expectOneCall("fake_write")
		.withParameter("key", "prefix/index")
		.ignoreOtherParameters()
		.andReturnValue(-ENOMEM);
	LONGS_EQUAL(-ENOMEM, metricfs_write(fs, data, sizeof(data), NULL));
	LONGS_EQUAL(2, metricfs_count(fs));
}

TEST(metricfs, iterate_ShouldCallCallback_WhenMetricsExist) {
	uint8_t buf[8];
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	expect_index_write("prefix/1", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));

	expect_peek("prefix/0", data, sizeof(data));
	expect_peek("prefix/1", data, sizeof(data));
	mock().expectOneCall("on_iterate").withParameter("id", 0);
	mock().expectOneCall("on_iterate").withParameter("id", 1);
	LONGS_EQUAL(0, metricfs_iterate(fs, on_iterate, NULL, buf, sizeof(buf), MAX_METRICS));
}

TEST(metricfs, clear_ShouldClearAllMetrics_WhenCalled) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	expect_index_write("prefix/1", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));

	outdex = 2;
	mock().expectOneCall("fake_clear")
		.withParameter("key", "prefix/0")
		.andReturnValue(0);
	mock().expectOneCall("fake_clear")
		.withParameter("key", "prefix/1")
		.andReturnValue(0);
	mock().expectOneCall("fake_write")
		.withParameter("key", "prefix/outdex")
		.withMemoryBufferParameter("value", (const uint8_t *)&outdex, sizeof(outdex))
		.andReturnValue((int)sizeof(outdex));
	LONGS_EQUAL(0, metricfs_clear(fs));
	LONGS_EQUAL(0, metricfs_count(fs));
}

TEST(metricfs, ShouldNotIncreaseOutdex_WhenDeleteFailed) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	expect_index_write("prefix/1", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));

	mock().expectOneCall("fake_clear")
		.withParameter("key", "prefix/0")
		.andReturnValue(-ENOMEM);
	LONGS_EQUAL(-ENOMEM, metricfs_del_first(fs, NULL));
	LONGS_EQUAL(2, metricfs_count(fs));
}

/* when item deleted successfully, but outdex write failed */
TEST(metricfs, ShouldIncreaseOutdex_WhenReadReturnNOENT) {
	uint8_t data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
	expect_index_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));
	expect_index_write("prefix/1", data, sizeof(data));
	LONGS_EQUAL(0, metricfs_write(fs, data, sizeof(data), NULL));

	mock().expectOneCall("fake_read")
		.withParameter("key", "prefix/0")
		.ignoreOtherParameters()
		.andReturnValue(-ENOENT);
	expect_outdex_write("prefix/0", data, sizeof(data));
	LONGS_EQUAL(-ENOENT, metricfs_read_first(fs, data, sizeof(data), NULL));
	LONGS_EQUAL(1, metricfs_count(fs));
}
