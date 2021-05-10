#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <string.h>
#include <time.h>

#include "libmcu/logging.h"
#include "libmcu/logging_storage.h"

static const char *TAG = "logging";

time_t time(time_t *ptr) {
	(void)ptr;
	return mock().actualCall(__func__).returnLongIntValueOrDefault(0);
}
static size_t storage_write(const void *data, size_t datasize) {
	return mock().actualCall(__func__)
		.withParameterOfType("logDataType", "data", data)
		.withParameter("datasize", datasize)
		.returnUnsignedLongIntValueOrDefault(datasize);
}
static size_t storage_read(void *buf, size_t bufsize) {
	return mock().actualCall(__func__)
		.returnUnsignedLongIntValueOrDefault(0);
}
static size_t storage_consume(size_t consume_size) {
	return mock().actualCall(__func__)
		.returnUnsignedLongIntValueOrDefault(consume_size);
}
static size_t storage_count(void) {
	return mock().actualCall(__func__)
		.returnUnsignedLongIntValueOrDefault(0);
}
static void tag_callback(const char *tag, logging_t min_log_level) {
	mock().actualCall(__func__)
		.withParameterOfType("stringType", "tag", tag)
		.withParameter("min_log_level", min_log_level);
}

static const logging_storage_t storage = {
	.write = storage_write,
	.read = storage_read,
	.consume = storage_consume,
	.count = storage_count,
};

class LogDataComparator : public MockNamedValueComparator
{
public:
	virtual bool isEqual(const void *object1, const void *object2)
	{
		size_t len = mock().getData("expectedSize")
			.getUnsignedLongIntValue();

		if (memcmp(object1, object2, len) != 0) {
			return false;
		}

		return true;
	}
	virtual SimpleString valueToString(const void* object)
	{
		return StringFrom(object);
	}
};

class StringComparator : public MockNamedValueComparator
{
public:
	virtual bool isEqual(const void *object1, const void *object2)
	{
		if (strcmp((const char *)object1, (const char *)object2) != 0) {
			return false;
		}

		return true;
	}
	virtual SimpleString valueToString(const void* object)
	{
		return StringFrom(object);
	}
};

TEST_GROUP(logging) {
	void setup(void) {
		static LogDataComparator logDataComparator;
		static StringComparator strComparator;
		mock().installComparator("logDataType", logDataComparator);
		mock().installComparator("stringType", strComparator);
		mock().ignoreOtherCalls();

		logging_init(&storage);
	}
	void teardown() {
		mock().checkExpectations();
		mock().removeAllComparatorsAndCopiers();
		mock().clear();
	}
};

TEST(logging, get_level_ShouldReturnVerbose_WhenInitialStateGiven) {
	LONGS_EQUAL(LOGGING_TYPE_VERBOSE, logging_get_level_current());
}
TEST(logging, get_level_ShouldReturnGlobalLogLevel_WhenTagFull) {
	const logging_context l1 = { .tag = "#1", };
	const logging_context l2 = { .tag = "#2", };
	const logging_context l3 = { .tag = "#3", };
	const logging_context l4 = { .tag = "#4", };
	const logging_context l5 = { .tag = "#5", };
	const logging_context l6 = { .tag = "#6", };
	const logging_context l7 = { .tag = "#7", };
	const logging_context l8 = { .tag = "#8", };
	logging_save(LOGGING_TYPE_INFO, &l1, "");
	logging_save(LOGGING_TYPE_INFO, &l2, "");
	logging_save(LOGGING_TYPE_INFO, &l3, "");
	logging_save(LOGGING_TYPE_INFO, &l4, "");
	logging_save(LOGGING_TYPE_INFO, &l5, "");
	logging_save(LOGGING_TYPE_INFO, &l6, "");
	logging_save(LOGGING_TYPE_INFO, &l7, "");
	logging_save(LOGGING_TYPE_INFO, &l8, "");
	logging_set_level("NEW", LOGGING_TYPE_ERROR);
	LONGS_EQUAL(LOGGING_TYPE_VERBOSE, logging_get_level("NEW"));
}
TEST(logging, get_level_global_ShouldReturnVerbose_WhenInitialStateGiven) {
	LONGS_EQUAL(LOGGING_TYPE_VERBOSE, logging_get_level_global());
}

TEST(logging, set_level_ShouldSetLogLevel_WhenLogInfoGiven) {
	logging_set_level_current(LOGGING_TYPE_INFO);
	LONGS_EQUAL(LOGGING_TYPE_INFO, logging_get_level_current());
}
TEST(logging, set_level_ShouldSetLogLevel_WhenLogErrorGiven) {
	logging_set_level_current(LOGGING_TYPE_ERROR);
	LONGS_EQUAL(LOGGING_TYPE_ERROR, logging_get_level_current());
}
TEST(logging, set_level_ShouldDoNothing_WhenInvalidParamGiven) {
	logging_set_level_current(LOGGING_TYPE_MAX);
	LONGS_EQUAL(LOGGING_TYPE_VERBOSE, logging_get_level_current());
}

TEST(logging, save_ShouldWriteLogWithMessage_WhenMessageGiven) {
	const uint8_t expected[] = {
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xfe, 0xca, 0xde, 0xc0, 0x00, 0x00, 0x00, 0x00,
		0xef, 0xbe, 0xed, 0xfe, 0x00, 0x00, 0x00, 0x00,
		0xb4, 0xd1, 0x02, 0x0e, 0x54, 0x68, 0x65, 0x20,
		0x66, 0x69, 0x72, 0x73, 0x74, 0x20, 0x74, 0x65,
		0x73, 0x74 };
	mock().expectOneCall("time").andReturnValue(1);
	mock().expectOneCall("storage_write")
		.withParameterOfType("logDataType", "data", expected)
		.withParameter("datasize", 42);
	mock().setData("expectedSize", 42);

	const logging_context l = {
		.tag = "mytag",
		.pc = (const void *)0xc0decafe,
		.lr = (const void *)0xfeedbeef,
	};
	logging_save(LOGGING_TYPE_INFO, &l, "The first test");
}
TEST(logging, save_ShouldNotWriteLog_WhenLogLevelIsError) {
	mock().expectNoCall("storage_write");
	logging_set_level_current(LOGGING_TYPE_ERROR);
	info("Should not write");
}
TEST(logging, save_ShouldNotWriteLog_WhenGlobalLogLevelIsError) {
	mock().expectNoCall("storage_write");
	logging_set_level_global(LOGGING_TYPE_ERROR);
	info("Should not write");
}
IGNORE_TEST(logging, save_ShouldTakeCareOfNullPointer_WhenNullMessageDelivered) {
	const logging_context l = { .tag = "#1", };
	size_t written = logging_save(LOGGING_TYPE_INFO, &l, NULL);
	uint8_t buf[128];
	size_t bytes_read = logging_read(buf, sizeof(buf));
	LONGS_EQUAL(written, bytes_read);
}
TEST(logging, save_ShouldReturnZero_WhenLogLevelIsLowerThanMinLogLevel) {
	const logging_context default_logctx = { .tag = TAG, };
	logging_set_level_current(LOGGING_TYPE_ERROR);
	LONGS_EQUAL(0, logging_save(LOGGING_TYPE_INFO, &default_logctx, ""));
}
TEST(logging, save_ShouldReturnZero_WhenLogLevelIsInvalid) {
	const logging_context default_logctx = { .tag = TAG, };
	LONGS_EQUAL(0, logging_save(LOGGING_TYPE_MAX, &default_logctx, ""));
}
TEST(logging, save_ShouldReturnWrittenSize) {
	const logging_context default_logctx = { .tag = TAG, };
	LONGS_EQUAL(28, logging_save(LOGGING_TYPE_INFO, &default_logctx, ""));
}
TEST(logging, save_ShouldParseFormattedString) {
	const uint8_t expected[] = {
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xa5, 0xa5, 0x02, 0x11, 0x66, 0x6d, 0x74, 0x20,
		0x31, 0x32, 0x33, 0x3a, 0x20, 0x6d, 0x79, 0x73,
		0x74, 0x72, 0x69, 0x6e, 0x67 };
	mock().expectOneCall("time").andReturnValue(1);
	mock().expectOneCall("storage_write")
		.withParameterOfType("logDataType", "data", expected)
		.withParameter("datasize", 45);
	mock().setData("expectedSize", 45);

	const logging_context default_logctx = { .tag = TAG, };
	logging_save(LOGGING_TYPE_INFO, &default_logctx,
			"fmt %d: %s", 123, "mystring");
}

TEST(logging, stringify_ShouldReturnString_WhenLogGiven) {
	const uint8_t fixed_log[] = {
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xfe, 0xca, 0xde, 0xc0, 0x00, 0x00, 0x00, 0x00,
		0xef, 0xbe, 0xed, 0xfe, 0x00, 0x00, 0x00, 0x00,
		0xb4, 0xd1, 0x02, 0x0e, 0x54, 0x68, 0x65, 0x20,
		0x66, 0x69, 0x72, 0x73, 0x74, 0x20, 0x74, 0x65,
		0x73, 0x74 };
	const char *expected =
		"1: [INFO] <0xc0decafe,0xfeedbeef> The first test";
	char buf[256];

	POINTERS_EQUAL(buf, logging_stringify(buf, sizeof(buf), fixed_log));
	STRNCMP_EQUAL(expected, buf, strlen(expected));
}

TEST(logging, count_tags_ShouldReturnZero_WhenInitialStateGiven) {
	LONGS_EQUAL(0, logging_count_tags());
}
TEST(logging, count_tags_ShouldReturnNumberOfTags) {
	const logging_context l1 = { .tag = "#1", };
	const logging_context l2 = { .tag = "#2", };
	logging_save(LOGGING_TYPE_INFO, &l1, "");
	LONGS_EQUAL(1, logging_count_tags());
	logging_save(LOGGING_TYPE_INFO, &l2, "");
	LONGS_EQUAL(2, logging_count_tags());
}

TEST(logging, iterate_tag_ShouldRunCallback) {
	const logging_context l1 = { .tag = "#1", };
	const logging_context l2 = { .tag = "#2", };
	logging_save(LOGGING_TYPE_INFO, &l1, "");
	logging_save(LOGGING_TYPE_INFO, &l2, "");
	logging_set_level("#1", LOGGING_TYPE_INFO);
	logging_set_level("#2", LOGGING_TYPE_ERROR);

	mock().expectOneCall("tag_callback")
		.withParameterOfType("stringType", "tag", "#1")
		.withParameter("min_log_level", LOGGING_TYPE_INFO);
	mock().expectOneCall("tag_callback")
		.withParameterOfType("stringType", "tag", "#2")
		.withParameter("min_log_level", LOGGING_TYPE_ERROR);

	logging_iterate_tag(tag_callback);
}

TEST(logging, count_ShouldReturnZero_WhenNologsSaved) {
	LONGS_EQUAL(0, logging_count());
}
TEST(logging, count_ShouldReturnTotalNumberOfLogsSaved) {
	mock().expectOneCall("storage_count").andReturnValue(4);
	mock().expectNCalls(4, "storage_write").ignoreOtherParameters();

	info("1");
	info("2");
	info("3");
	info("4");

	LONGS_EQUAL(4, logging_count());
}
TEST(logging, count_ShouldReturnTotalNumberOfLogsSaved_WhenMultiTagsGiven) {
	mock().expectOneCall("storage_count").andReturnValue(3);
	mock().expectNCalls(3, "storage_write").ignoreOtherParameters();

	const logging_context l1 = { .tag = "#1", };
	const logging_context l2 = { .tag = "#2", };
	const logging_context l3 = { .tag = "#3", };
	logging_save(LOGGING_TYPE_INFO, &l1, "");
	logging_save(LOGGING_TYPE_INFO, &l2, "");
	logging_save(LOGGING_TYPE_INFO, &l3, "");

	LONGS_EQUAL(3, logging_count());
}

TEST(logging, read_ShouldReturnZero_WhenBufIsNull) {
	LONGS_EQUAL(0, logging_read(NULL, 100));
}
TEST(logging, read_ShouldReturnZero_WhenNoLogs) {
	uint8_t buf[128];
	LONGS_EQUAL(0, logging_read(buf, sizeof(buf)));
}
TEST(logging, read_ShouldReturnZero_WhenBufSizeNotEnough) {
	uint8_t buf[1];
	debug("");
	LONGS_EQUAL(0, logging_read(buf, sizeof(buf)));
}
TEST(logging, read_ShouldReturnLogSize) {
	const logging_context default_logctx = { .tag = TAG, };
	uint8_t buf[128];
	size_t bytes_written = logging_save(LOGGING_TYPE_INFO, &default_logctx,
			NULL);
	mock().expectOneCall("storage_read").andReturnValue(bytes_written);
	LONGS_EQUAL(bytes_written, logging_read(buf, sizeof(buf)));
}

TEST(logging, LOGGING_TAG_ShouldReturnCurrentTag) {
	STRCMP_EQUAL(TAG, LOGGING_TAG);
}
