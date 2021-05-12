#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include <string.h>

#include "libmcu/pubsub.h"

static const char *testopic = "group/user/id";

static void callback(void *context, const void *msg, size_t msglen) {
	static uint8_t copied[1024];
	memcpy(copied, msg, msglen);
	mock().setData("received_message", copied);

	mock().actualCall(__func__)
		.withPointerParameter("context", context)
		.withConstPointerParameter("msg", msg)
		.withParameter("msglen", msglen);
}

TEST_GROUP(PubSub) {
	pubsub_subscribe_static_t subs[PUBSUB_MIN_SUBSCRIPTION_CAPACITY];

	void setup(void) {
		mock().ignoreOtherCalls();

		pubsub_init();
	}
	void teardown() {
		pubsub_deinit();

		mock().checkExpectations();
		mock().clear();
	}
};

// create() is doing nothing but a place holder for pubsub_tiny.
TEST(PubSub, create_ShouldReturnSuccess) {
	const char *fixed_topic = "mytopic";
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_create(fixed_topic));
	pubsub_destroy(fixed_topic);
}

TEST(PubSub, destroy_ShouldReturnSuccess) {
	const char *fixed_topic = "mytopic";
	pubsub_create(fixed_topic);
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_destroy(fixed_topic));
}

TEST(PubSub, subscribe_ShouldReturnSubscriptionHandle) {
	pubsub_subscribe_t sub = pubsub_subscribe(testopic, callback, NULL);
	CHECK(sub != NULL);
	pubsub_unsubscribe(sub);
}

TEST(PubSub, subscribe_static_ShouldReturnSubscriptionHandle) {
	pubsub_subscribe_static_t sub;
	pubsub_subscribe_t handle =
		pubsub_subscribe_static(&sub, testopic, callback, NULL);
	POINTERS_EQUAL(&sub, handle);
	pubsub_unsubscribe(&sub);
}

TEST(PubSub, subscribe_ShouldReturnNull_WhenNullParamsGiven) {
	POINTERS_EQUAL(NULL, pubsub_subscribe(NULL, callback, NULL));
	POINTERS_EQUAL(NULL, pubsub_subscribe(testopic, NULL, NULL));
}

TEST(PubSub, subscribe_ShouldReturnNull_WhenAllocationFail) {
	cpputest_malloc_set_out_of_memory();
	POINTERS_EQUAL(NULL, pubsub_subscribe(testopic, callback, NULL));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(PubSub, subscribe_ShouldReturnSubsHandle_WhenNoMatchingTopicYetFound) {
	pubsub_subscribe_t handle =
		pubsub_subscribe("unknown topic", callback, NULL);
	CHECK(handle != NULL);
	pubsub_unsubscribe(handle);
}

TEST(PubSub, subscribe_static_ShouldReturnNull_WhenNullParamsGiven) {
	pubsub_subscribe_static_t sub;
	POINTERS_EQUAL(NULL,
			pubsub_subscribe_static(&sub, NULL, callback, NULL));
	POINTERS_EQUAL(NULL,
			pubsub_subscribe_static(&sub, testopic, NULL, NULL));
}

TEST(PubSub, subscribe_static_ShouldReturnSubsHandle_WhenNoMatchingTopicYetFound) {
	pubsub_subscribe_static_t sub;
	pubsub_subscribe_t handle =
		pubsub_subscribe_static(&sub, "unknown topic", callback, NULL);
	POINTERS_EQUAL(&sub, handle);
	pubsub_unsubscribe(handle);
}

TEST(PubSub, unsubscribe_static_ShouldReturnSuccess) {
	pubsub_subscribe_static_t sub;
	pubsub_subscribe_t handle =
		pubsub_subscribe_static(&sub, testopic, callback, NULL);
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_unsubscribe(handle));
}

TEST(PubSub, unsubscribe_ShouldReturnSuccess) {
	pubsub_subscribe_t sub = pubsub_subscribe(testopic, callback, NULL);
	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_unsubscribe(sub));
}

TEST(PubSub, unsubscribe_ShouldReturnInvalidParams_WhenNullParamsGiven) {
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_unsubscribe(NULL));
}

TEST(PubSub, publish_ShouldReturnSuccess) {
	LONGS_EQUAL(PUBSUB_SUCCESS,
			pubsub_publish(testopic, testopic, strlen(testopic)));
}

TEST(PubSub, publish_ShouldReturnInvaludParams_WhenNullParamsGiven) {
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_publish(NULL, "message", 7));
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_publish(testopic, NULL, 7));
}

TEST(PubSub, publish_ShouldReturnSuccessAndCallCallback) {
	const char *fixed_messaged = "message";

	pubsub_subscribe_t sub1 = pubsub_subscribe(testopic, callback, NULL);
	pubsub_subscribe_t sub2 = pubsub_subscribe(testopic, callback, NULL);
	pubsub_subscribe_t sub3 = pubsub_subscribe(testopic, callback, NULL);

	mock().expectNCalls(3, "callback")
		.withPointerParameter("context", NULL)
		.withConstPointerParameter("msg", fixed_messaged)
		.withParameter("msglen", strlen(fixed_messaged));

	LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_publish(testopic, fixed_messaged,
				strlen(fixed_messaged)));

	pubsub_unsubscribe(sub1);
	pubsub_unsubscribe(sub2);
	pubsub_unsubscribe(sub3);
}

TEST(PubSub, count_ShouldReturnInvalidParam_WhenNullTopicGiven) {
	LONGS_EQUAL(PUBSUB_INVALID_PARAM, pubsub_count(NULL));
}

TEST(PubSub, count_ShouldReturnZero_WhenNoSubscriberRegistered) {
	const char *fixed_topic = "mytopic";
	pubsub_create(fixed_topic);
	LONGS_EQUAL(0, pubsub_count(fixed_topic));
	pubsub_destroy(fixed_topic);
}

TEST(PubSub, count_ShouldReturnOne_WhenOneSubscriberRegistered) {
	pubsub_subscribe_t sub = pubsub_subscribe(testopic, callback, NULL);
	LONGS_EQUAL(1, pubsub_count(testopic));
	pubsub_unsubscribe(sub);
}

TEST(PubSub, count_ShouldReturnThree_WhenThreeSubscriberRegistered) {
	pubsub_subscribe_t sub1 = pubsub_subscribe(testopic, callback, NULL);
	pubsub_subscribe_t sub2 = pubsub_subscribe(testopic, callback, NULL);
	pubsub_subscribe_t sub3 = pubsub_subscribe(testopic, callback, NULL);

	LONGS_EQUAL(3, pubsub_count(testopic));

	pubsub_unsubscribe(sub1);
	pubsub_unsubscribe(sub2);
	pubsub_unsubscribe(sub3);
}

TEST(PubSub, subscribe_ShouldExpandCapacity_WhenSubscriptionsFull) {
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		CHECK(pubsub_subscribe(testopic, callback, NULL) != NULL);
	}
	CHECK(pubsub_subscribe(testopic, callback, NULL) != NULL);
}

TEST(PubSub, subscribe_ShouldReturnNull_WhenExpansionFailDueToOOM) {
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		CHECK(pubsub_subscribe(testopic, callback, NULL) != NULL);
	}
	cpputest_malloc_set_out_of_memory_countdown(2);
	POINTERS_EQUAL(NULL, pubsub_subscribe(testopic, callback, NULL));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(PubSub, subscribe_static_ShouldExpandCapacity_WhenSubscriptionsFull) {
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		CHECK(pubsub_subscribe_static(&subs[i], testopic, callback, NULL)
				!= NULL);
	}
	pubsub_subscribe_static_t extra_sub;
	CHECK(pubsub_subscribe_static(&extra_sub, testopic, callback, NULL)
			!= NULL);
	pubsub_unsubscribe(&extra_sub);
}

TEST(PubSub, subscribe_static_ShouldReturnNull_WhenExpansionFailDueToOOM) {
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		CHECK(pubsub_subscribe_static(&subs[i], testopic, callback, NULL)
				!= NULL);
	}
	pubsub_subscribe_static_t extra_sub;
	cpputest_malloc_set_out_of_memory();
	POINTERS_EQUAL(NULL, pubsub_subscribe_static(&extra_sub,
				testopic, callback, NULL));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(PubSub, unsubscribe_ShouldReturnNoExist_WhenNotRegisteredSubGiven) {
	pubsub_subscribe_static(&subs[0], "#", callback, NULL);
	pubsub_unsubscribe(&subs[0]);
	LONGS_EQUAL(PUBSUB_NO_EXIST_SUBSCRIBER, pubsub_unsubscribe(&subs[0]));
}

TEST(PubSub, unsubscribe_ShouldReturnSuccess_WhenShrinkFailDueToOOM) {
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		pubsub_subscribe_static(&subs[i], testopic, callback, NULL);
	}
	pubsub_subscribe_static_t extra_sub;
	pubsub_subscribe_static(&extra_sub, testopic, callback, NULL);
	pubsub_unsubscribe(&extra_sub);

	cpputest_malloc_set_out_of_memory();
	for (int i = 0; i < PUBSUB_MIN_SUBSCRIPTION_CAPACITY; i++) {
		LONGS_EQUAL(PUBSUB_SUCCESS, pubsub_unsubscribe(&subs[i]));
	}
	cpputest_malloc_set_not_out_of_memory();
}

TEST(PubSub, publish_ShouldPublish_WhenMultiLevelWildcardSubsGiven) {
	const char *fixed_messaged = "message";

	pubsub_subscribe_t sub1 = pubsub_subscribe("#", callback, NULL);
	pubsub_subscribe_t sub2 = pubsub_subscribe("group/#", callback, NULL);
	pubsub_subscribe_t sub3 =
		pubsub_subscribe("group/user/#", callback, NULL);

	mock().expectNCalls(3, "callback")
		.withPointerParameter("context", NULL)
		.withConstPointerParameter("msg", fixed_messaged)
		.withParameter("msglen", strlen(fixed_messaged));
	pubsub_publish(testopic, fixed_messaged, strlen(fixed_messaged));

	pubsub_unsubscribe(sub1);
	pubsub_unsubscribe(sub2);
	pubsub_unsubscribe(sub3);
}

TEST(PubSub, publish_ShouldNotPublish_WhenWrongMultiLevelWildcardSubsGiven) {
	const char *fixed_messaged = "message";
	pubsub_subscribe_t sub1 =
		pubsub_subscribe("group/user/id/#", callback, NULL);
	pubsub_subscribe_t sub2 =
		pubsub_subscribe("group/user/id/a", callback, NULL);
	pubsub_subscribe_t sub3 =
		pubsub_subscribe("group/admin/#", callback, NULL);
	pubsub_subscribe_t sub4 =
		pubsub_subscribe("group", callback, NULL);

	mock().expectNoCall("callback");
	pubsub_publish(testopic, fixed_messaged, strlen(fixed_messaged));

	pubsub_unsubscribe(sub1);
	pubsub_unsubscribe(sub2);
	pubsub_unsubscribe(sub3);
	pubsub_unsubscribe(sub4);
}

TEST(PubSub, publish_ShouldPublish_WhenSingleLevelWildcardSubsGiven) {
	const char *fixed_messaged = "message";
	pubsub_subscribe_t sub1 = pubsub_subscribe("+/user/id", callback, NULL);
	pubsub_subscribe_t sub2 = pubsub_subscribe("group/+/id", callback, NULL);
	pubsub_subscribe_t sub3 = pubsub_subscribe("+/#", callback, NULL);
	pubsub_subscribe_t sub4 = pubsub_subscribe("+/user/#", callback, NULL);
	pubsub_subscribe_t sub5 = pubsub_subscribe("group/user/+", callback, NULL);

	mock().expectNCalls(5, "callback")
		.withPointerParameter("context", NULL)
		.withConstPointerParameter("msg", fixed_messaged)
		.withParameter("msglen", strlen(fixed_messaged));
	pubsub_publish(testopic, fixed_messaged, strlen(fixed_messaged));

	pubsub_unsubscribe(sub1);
	pubsub_unsubscribe(sub2);
	pubsub_unsubscribe(sub3);
	pubsub_unsubscribe(sub4);
	pubsub_unsubscribe(sub5);
}

TEST(PubSub, publish_ShouldNotPublish_WhenWrongSingleLevelWildcardSubsGiven) {
	const char *fixed_messaged = "message";
	pubsub_subscribe_t sub1 = pubsub_subscribe("+", callback, NULL);
	pubsub_subscribe_t sub2 = pubsub_subscribe("group/+", callback, NULL);
	pubsub_subscribe_t sub3 = pubsub_subscribe("+/user", callback, NULL);

	mock().expectNoCall("callback");
	pubsub_publish(testopic, fixed_messaged, strlen(fixed_messaged));

	pubsub_unsubscribe(sub1);
	pubsub_unsubscribe(sub2);
	pubsub_unsubscribe(sub3);
}

TEST(PubSub, publish_ShouldPublish_WhenWildcardSubsGiven) {
	const char *fixed_messaged = "message";
	pubsub_subscribe_t sub = pubsub_subscribe("group/+/user", callback, NULL);

	mock().expectNCalls(3, "callback")
		.withPointerParameter("context", NULL)
		.withConstPointerParameter("msg", fixed_messaged)
		.withParameter("msglen", strlen(fixed_messaged));

	pubsub_publish("group/a/user", fixed_messaged, strlen(fixed_messaged));
	pubsub_publish("group/b/user", fixed_messaged, strlen(fixed_messaged));
	pubsub_publish("group/c/user", fixed_messaged, strlen(fixed_messaged));

	pubsub_unsubscribe(sub);
}

TEST(PubSub, publish_wildcardExtraTest) {
	const char *fixed_messaged = "message";
	pubsub_subscribe_t sub = pubsub_subscribe("+", callback, NULL);

	mock().expectOneCall("callback")
		.withPointerParameter("context", NULL)
		.withConstPointerParameter("msg", fixed_messaged)
		.withParameter("msglen", strlen(fixed_messaged));
	pubsub_publish("a/b", fixed_messaged, strlen(fixed_messaged));

	mock().expectNoCall("callback");
	pubsub_publish("abc", fixed_messaged, strlen(fixed_messaged));

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
