#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include <string.h>
#include "pubsub.h"

static int callback_count;
static uint8_t message_spy[128];
static size_t message_length_spy;
static void callback(void *context, const void *msg, size_t msglen)
{
	callback_count++;
	message_length_spy = msglen;
	memcpy(message_spy, msg, msglen);
}

TEST_GROUP(PubSub) {
	const char *topic = "default/name";

	void setup(void) {
		pubsub_create(topic);

		callback_count = 0;
		message_length_spy = 0;
		memset(message_spy, 0, sizeof(message_spy));
	}
	void teardown() {
		pubsub_destroy(topic);
	}
};

TEST(PubSub, create_ShouldReturnSuccess) {
	const char *mytopic = "mytopic";
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_create(mytopic));
	LONGS_EQUAL(0, pubsub_count(mytopic));
	pubsub_destroy(mytopic);
}

TEST(PubSub, create_ShouldReturnInvaludParam_WhenNullTopicGiven) {
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_create(NULL));
}

TEST(PubSub, create_ShouldReturnNoMemory_WhenAllocationFailForTopic) {
	const char *mytopic = "mytopic";
	cpputest_malloc_set_out_of_memory();
	LONGS_EQUAL(PUBSUB_NO_MEMORY, pubsub_create(mytopic));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(PubSub, create_ShouldReturnNoMemory_WhenAllocationFailForTopicName) {
	const char *mytopic = "mytopic";
	cpputest_malloc_set_out_of_memory_countdown(2);
	LONGS_EQUAL(PUBSUB_NO_MEMORY, pubsub_create(mytopic));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(PubSub, create_ShouldReturnExist_WhenExistingTopicGiven) {
	const char *mytopic = "mytopic";
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_create(mytopic));
	LONGS_EQUAL(PUBSUB_TOPIC_EXIST, pubsub_create(mytopic));
	pubsub_destroy(mytopic);
}

TEST(PubSub, create_ShouldTruncateTopic_WhenLenthyTopicGiven) {
	char fixed_topic[PUBSUB_TOPIC_NAME_MAXLEN+1];
	char mytopic[PUBSUB_TOPIC_NAME_MAXLEN * 2];
	const char *ctbl = "1234567890";
	for (size_t i = 0; i < sizeof(mytopic); i++) {
		mytopic[i] = ctbl[i % 10];
	}
	strncpy(fixed_topic, mytopic, sizeof(fixed_topic));
	fixed_topic[sizeof(fixed_topic)-1] = '\0';

	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_create(mytopic));
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_publish(mytopic, "message", 7));
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_publish(fixed_topic, "message", 7));
	pubsub_destroy(mytopic);
}

TEST(PubSub, destroy_ShouldReturnSuccess) {
	const char *mytopic = "mytopic";
	pubsub_create(mytopic);
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_destroy(mytopic));
}

TEST(PubSub, destroy_ShouldReturnInvaludParam_WhenNullTopicGiven) {
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_destroy(NULL));
}

TEST(PubSub, destroy_ShouldReturnTopicNotExist_WhenNoMatchingTopicFound) {
	LONGS_EQUAL(PUBSUB_TOPIC_NOT_EXIST, pubsub_destroy("mytopic"));
}

TEST(PubSub, destroy_ShouldRemoveAndDestroySubscriptionsRegisterd) {
	const char *mytopic = "mytopic";
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_create(mytopic));
	pubsub_subscribe(mytopic, callback, NULL);
	pubsub_subscribe(mytopic, callback, NULL);
	pubsub_subscribe(mytopic, callback, NULL);
	LONGS_EQUAL(3, pubsub_count(mytopic));
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_destroy(mytopic));
}

TEST(PubSub, subscribe_ShouldReturnHandler) {
	pubsub_subscribe_t *sub = pubsub_subscribe(topic, callback, NULL);
	CHECK(sub != NULL);
	LONGS_EQUAL(1, pubsub_count(topic));
	pubsub_unsubscribe(sub);
}

TEST(PubSub, subscribe_ShouldReturnNull_WhenNullParamsGiven) {
	POINTERS_EQUAL(NULL, pubsub_subscribe(NULL, callback, NULL));
	POINTERS_EQUAL(NULL, pubsub_subscribe(topic, NULL, NULL));
}

TEST(PubSub, unsubscribe_ShouldReturnSuccess) {
	pubsub_subscribe_t *sub = pubsub_subscribe(topic, callback, NULL);
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_unsubscribe(sub));
}

TEST(PubSub, unsubscribe_ShouldReturnInvalidParams_WhenNullParamsGiven) {
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_unsubscribe(NULL));
}

TEST(PubSub, publish_ShouldReturnSuccess) {
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_publish(topic, "message", 7));
}

TEST(PubSub, publish_ShouldReturnInvaludParams_WhenNullParamsGiven) {
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_publish(NULL, "message", 7));
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_publish(topic, NULL, 7));
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_publish(topic, "message", 0));
}

TEST(PubSub, publish_ShouldReturnSuccessAndCallCallback) {
	pubsub_subscribe_t *sub1 = pubsub_subscribe(topic, callback, NULL);
	pubsub_subscribe_t *sub2 = pubsub_subscribe(topic, callback, NULL);
	pubsub_subscribe_t *sub3 = pubsub_subscribe(topic, callback, NULL);
	LONGS_EQUAL(3, pubsub_count(topic));
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_publish(topic, "message", 7));
	LONGS_EQUAL(7, message_length_spy);
	MEMCMP_EQUAL("message", message_spy, message_length_spy);

	pubsub_unsubscribe(sub1);
	pubsub_unsubscribe(sub2);
	pubsub_unsubscribe(sub3);
}

TEST(PubSub, publish_ShouldReturnTopicNotExist_WhenNotRegisteredTopicGiven) {
	LONGS_EQUAL(PUBSUB_TOPIC_NOT_EXIST, pubsub_publish("tmp", "message", 7));
}

TEST(PubSub, count_ShouldReturnInvalidParam_WhenNullTopicGiven) {
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_count(NULL));
}

TEST(PubSub, count_ShouldReturnTopicNotExist_WhenNotRegisteredTopicGiven) {
	LONGS_EQUAL(PUBSUB_TOPIC_NOT_EXIST, pubsub_count("tmp"));
}

TEST(PubSub, stringify_ShouldReturnStrings_WhenErrorCodeGiven) {
	STRCMP_EQUAL("success", pubsub_stringify_error(PUBSUB_SUCCESS));
	STRCMP_EQUAL("error", pubsub_stringify_error(PUBSUB_ERROR));
	STRCMP_EQUAL("topic already exist", pubsub_stringify_error(PUBSUB_TOPIC_EXIST));
	STRCMP_EQUAL("topic not exist", pubsub_stringify_error(PUBSUB_TOPIC_NOT_EXIST));
	STRCMP_EQUAL("no memory", pubsub_stringify_error(PUBSUB_NO_MEMORY));
	STRCMP_EQUAL("invalid parameters", pubsub_stringify_error(PUBSUB_INVALID_PARAM));
	STRCMP_EQUAL("subscriber already exist", pubsub_stringify_error(PUBSUB_SUBSCRIBERS_EXIST));
}
