#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include <string.h>
#include "libmcu/pubsub.h"

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
	const char *topic = "group/user/id";
	pubsub_subscribe_t subs[PUBSUB_MIN_SUBSCRIPTION_CAPACITY];

	void setup(void) {
		callback_count = 0;
		message_length_spy = 0;
		memset(message_spy, 0, sizeof(message_spy));

		pubsub_init();
	}
	void teardown() {
		pubsub_deinit();
	}
};

TEST(PubSub, create_ShouldReturnSuccess) {
	const char *mytopic = "mytopic";
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_create(mytopic));
	LONGS_EQUAL(0, pubsub_count(mytopic));
	pubsub_destroy(mytopic);
}

TEST(PubSub, destroy_ShouldReturnSuccess) {
	const char *mytopic = "mytopic";
	pubsub_create(mytopic);
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_destroy(mytopic));
}

TEST(PubSub, subscribe_ShouldReturnSubscriptionHandle) {
	pubsub_subscribe_t *sub = pubsub_subscribe(topic, callback, NULL);
	CHECK(sub != NULL);
	LONGS_EQUAL(1, pubsub_count(topic));
	pubsub_unsubscribe(sub);
}

TEST(PubSub, subscribe_ShouldReturnNull_WhenNullParamsGiven) {
	POINTERS_EQUAL(NULL, pubsub_subscribe(NULL, callback, NULL));
	POINTERS_EQUAL(NULL, pubsub_subscribe(topic, NULL, NULL));
}

TEST(PubSub, subscribe_ShouldReturnNull_WhenAllocationFail) {
	cpputest_malloc_set_out_of_memory();
	POINTERS_EQUAL(NULL, pubsub_subscribe(topic, callback, NULL));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(PubSub, subscribe_ShouldReturnSubsHandle_WhenNoMatchingTopicYetFound) {
	CHECK(pubsub_subscribe("unknown topic", callback, NULL) != NULL);
}

TEST(PubSub, subscribe_static_ShouldReturnNull_WhenNullParamsGiven) {
	pubsub_subscribe_t sub;
	POINTERS_EQUAL(NULL, pubsub_subscribe_static(&sub, NULL, callback, NULL));
	POINTERS_EQUAL(NULL, pubsub_subscribe_static(&sub, topic, NULL, NULL));
}

TEST(PubSub, subscribe_static_ShouldReturnSubsHandle_WhenNoMatchingTopicYetFound) {
	pubsub_subscribe_t sub;
	CHECK(pubsub_subscribe_static(&sub, "unknown topic", callback, NULL)
			!= NULL);
	pubsub_unsubscribe(&sub);
}

TEST(PubSub, subscribe_static_ShouldReturnSubscriptionHandle) {
	pubsub_subscribe_t sub;
	pubsub_subscribe_t *p =
		pubsub_subscribe_static(&sub, topic, callback, NULL);
	CHECK(p != NULL);
	LONGS_EQUAL(1, pubsub_count(topic));
	pubsub_unsubscribe(p);
}

TEST(PubSub, unsubscribe_static_ShouldReturnSuccess) {
	pubsub_subscribe_t sub;
	pubsub_subscribe_static(&sub, topic, callback, NULL);
	LONGS_EQUAL(1, pubsub_count(topic));
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_unsubscribe(&sub));
	LONGS_EQUAL(0, pubsub_count(topic));
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

TEST(PubSub, count_ShouldReturnInvalidParam_WhenNullTopicGiven) {
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_count(NULL));
}

TEST(PubSub, subscribe_ShouldExpandCapacity_WhenSubscriptionsFull) {
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		CHECK(pubsub_subscribe(topic, callback, NULL) != NULL);
	}
	CHECK(pubsub_subscribe(topic, callback, NULL) != NULL);
}

TEST(PubSub, subscribe_ShouldReturnNull_WhenExpansionFailDueToOOM) {
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		CHECK(pubsub_subscribe(topic, callback, NULL) != NULL);
	}
	cpputest_malloc_set_out_of_memory_countdown(2);
	POINTERS_EQUAL(NULL, pubsub_subscribe(topic, callback, NULL));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(PubSub, subscribe_static_ShouldExpandCapacity_WhenSubscriptionsFull) {
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		CHECK(pubsub_subscribe_static(&subs[i], topic, callback, NULL)
				!= NULL);
	}
	pubsub_subscribe_t extra_sub;
	CHECK(pubsub_subscribe_static(&extra_sub, topic, callback, NULL) != NULL);
	pubsub_unsubscribe(&extra_sub);
}

TEST(PubSub, subscribe_static_ShouldReturnNull_WhenExpansionFailDueToOOM) {
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		CHECK(pubsub_subscribe_static(&subs[i], topic, callback, NULL)
				!= NULL);
	}
	pubsub_subscribe_t extra_sub;
	cpputest_malloc_set_out_of_memory();
	POINTERS_EQUAL(NULL, pubsub_subscribe_static(&extra_sub,
				topic, callback, NULL));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(PubSub, unsubscribe_ShouldReturnNoExist_WhenNotRegisteredSubGiven) {
	pubsub_subscribe_static(&subs[0], "#", callback, NULL);
	pubsub_unsubscribe(&subs[0]);
	LONGS_EQUAL(PUBSUB_NO_EXIST_SUBSCRIBER, pubsub_unsubscribe(&subs[0]));
}

TEST(PubSub, unsubscribe_ShouldReturnSuccess_WhenShrinkFailDueToOOM) {
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		pubsub_subscribe_static(&subs[i], topic, callback, NULL);
	}
	pubsub_subscribe_t extra_sub;
	pubsub_subscribe_static(&extra_sub, topic, callback, NULL);
	pubsub_unsubscribe(&extra_sub);

	cpputest_malloc_set_out_of_memory();
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_unsubscribe(&subs[i]));
	}
	cpputest_malloc_set_not_out_of_memory();
}

TEST(PubSub, publish_ShouldPublish_WhenMultiLevelWildcardSubsGiven) {
	pubsub_subscribe_t *sub1 = pubsub_subscribe("#", callback, NULL);
	pubsub_subscribe_t *sub2 = pubsub_subscribe("group/#", callback, NULL);
	pubsub_subscribe_t *sub3 =
		pubsub_subscribe("group/user/#", callback, NULL);

	LONGS_EQUAL(3, pubsub_count(topic));
	pubsub_publish(topic, "message", 7);
	LONGS_EQUAL(7, message_length_spy);
	MEMCMP_EQUAL("message", message_spy, message_length_spy);
	LONGS_EQUAL(3, callback_count);

	pubsub_unsubscribe(sub1);
	pubsub_unsubscribe(sub2);
	pubsub_unsubscribe(sub3);
}

TEST(PubSub, publish_ShouldNotPublish_WhenWrongMultiLevelWildcardSubsGiven) {
	pubsub_subscribe_t *sub1 =
		pubsub_subscribe("group/user/id/#", callback, NULL);
	pubsub_subscribe_t *sub2 =
		pubsub_subscribe("group/user/id/a", callback, NULL);
	pubsub_subscribe_t *sub3 =
		pubsub_subscribe("group/admin/#", callback, NULL);
	pubsub_subscribe_t *sub4 =
		pubsub_subscribe("group", callback, NULL);

	LONGS_EQUAL(0, pubsub_count(topic));

	pubsub_unsubscribe(sub1);
	pubsub_unsubscribe(sub2);
	pubsub_unsubscribe(sub3);
	pubsub_unsubscribe(sub4);
}

TEST(PubSub, publish_ShouldPublish_WhenSingleLevelWildcardSubsGiven) {
	pubsub_subscribe_t *sub1 =
		pubsub_subscribe("+/user/id", callback, NULL);
	pubsub_subscribe_t *sub2 =
		pubsub_subscribe("group/+/id", callback, NULL);
	pubsub_subscribe_t *sub3 =
		pubsub_subscribe("+/#", callback, NULL);
	pubsub_subscribe_t *sub4 =
		pubsub_subscribe("+/user/#", callback, NULL);
	pubsub_subscribe_t *sub5 =
		pubsub_subscribe("group/user/+", callback, NULL);

	LONGS_EQUAL(5, pubsub_count(topic));

	pubsub_unsubscribe(sub1);
	pubsub_unsubscribe(sub2);
	pubsub_unsubscribe(sub3);
	pubsub_unsubscribe(sub4);
	pubsub_unsubscribe(sub5);
}

TEST(PubSub, publish_ShouldNotPublish_WhenWrongSingleLevelWildcardSubsGiven) {
	pubsub_subscribe_t *sub1 =
		pubsub_subscribe("+", callback, NULL);
	pubsub_subscribe_t *sub2 =
		pubsub_subscribe("group/+", callback, NULL);
	pubsub_subscribe_t *sub3 =
		pubsub_subscribe("+/user", callback, NULL);

	LONGS_EQUAL(0, pubsub_count(topic));

	pubsub_unsubscribe(sub1);
	pubsub_unsubscribe(sub2);
	pubsub_unsubscribe(sub3);
}

TEST(PubSub, publish_ShouldPublish_WhenWildcardSubsGiven) {
	pubsub_subscribe_t *sub = pubsub_subscribe("group/+/user", callback, NULL);
	pubsub_publish("group/a/user", "message", 7);
	pubsub_publish("group/b/user", "message", 7);
	pubsub_publish("group/c/user", "message", 7);

	LONGS_EQUAL(3, callback_count);

	pubsub_unsubscribe(sub);
}

TEST(PubSub, publish_wildcardExtraTest) {
	pubsub_subscribe_t *sub = pubsub_subscribe("+", callback, NULL);

	pubsub_publish("abc", "message", 7);
	LONGS_EQUAL(1, callback_count);
	pubsub_publish("a/b", "message", 7);
	LONGS_EQUAL(1, callback_count);

	pubsub_unsubscribe(sub);
}

TEST(PubSub, stringify_ShouldReturnStrings_WhenErrorCodeGiven) {
	STRCMP_EQUAL("success", pubsub_stringify_error(PUBSUB_SUCCESS));
	STRCMP_EQUAL("error", pubsub_stringify_error(PUBSUB_ERROR));
	STRCMP_EQUAL("exist topic", pubsub_stringify_error(PUBSUB_EXIST_TOPIC));
	STRCMP_EQUAL("no exist topic",
			pubsub_stringify_error(PUBSUB_NO_EXIST_TOPIC));
	STRCMP_EQUAL("no memory", pubsub_stringify_error(PUBSUB_NO_MEMORY));
	STRCMP_EQUAL("invalid parameters",
			pubsub_stringify_error(PUBSUB_INVALID_PARAM));
	STRCMP_EQUAL("exist subscriber",
			pubsub_stringify_error(PUBSUB_EXIST_SUBSCRIBER));
	STRCMP_EQUAL("no exist subscriber",
			pubsub_stringify_error(PUBSUB_NO_EXIST_SUBSCRIBER));
}
