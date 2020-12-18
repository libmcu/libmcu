#include "libmcu/pubsub.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

#include "libmcu/list.h"
#include "libmcu/compiler.h"
#include "libmcu/logging.h"

#if !defined(PUBSUB_TOPIC_DESTROY_MESSAGE)
#define PUBSUB_TOPIC_DESTROY_MESSAGE		"topic destroyed"
#endif
#if !defined(PUBSUB_MIN_SUBSCRIPTION_CAPACITY)
#define PUBSUB_MIN_SUBSCRIPTION_CAPACITY	1
#endif

/* NOTE: It sets the least significant bit of `subscriber->context` to
 * differentiate static subscriber from one created dynamically. */
#define GET_SUBSCRIBER_CONTEXT(obj)		\
	(void *)((uintptr_t)(obj)->context & ~1UL)
#define IS_SUBSCRIBER_STATIC(obj)		\
	((uintptr_t)(obj)->context & 1UL)
#define GET_CONTEXT_STATIC(ctx)			\
	(void *)((uintptr_t)(ctx) | 1UL)

typedef struct {
	char *name;
	struct list pubsub_node; // list entry for the pubsub_list

	const struct subscribe_s **subscriptions; // dynamically allocated array
	uint8_t max_subscriptions;
	uint8_t nr_subscriptions;
} topic_t;

typedef struct subscribe_s {
	const char *topic_filter;
	pubsub_callback_t callback;
	void *context;
	intptr_t _placeholder_for_compatibility;
} subscribe_t;
LIBMCU_STATIC_ASSERT(sizeof(subscribe_t) == sizeof(pubsub_subscribe_t),
	"The size of public and private subscribe data type must be the same.");

static struct {
	struct list pubsub_list;
	pthread_mutex_t pubsub_list_lock;
	volatile bool initialized;
} m;

static void add_topic(topic_t *topic)
{
	list_add(&topic->pubsub_node, &m.pubsub_list);
}

static void remove_topic(topic_t *topic)
{
	list_del(&topic->pubsub_node, &m.pubsub_list);
}

static topic_t *find_topic(const char *topic_name)
{
	topic_t *topic = NULL;

	struct list *i;
	list_for_each(i, &m.pubsub_list) {
		topic = list_entry(i, topic_t, pubsub_node);
		if (strncmp(topic_name, topic->name,
					PUBSUB_TOPIC_NAME_MAXLEN) == 0) {
			break;
		} else {
			topic = NULL;
		}
	}

	return topic;
}

static bool is_filtering_done(const char * const filter, const char * const topic)
{
	if (*filter != '\0') {
		return false;
	}
	if (*topic != '\0' && *topic != '/') {
		return false;
	}
	if (*topic != '\0' && topic[1] != '\0') {
		return false;
	}
	return true;
}

static bool is_topic_matched_with(const char * const filter,
		const char * const topic)
{
	if (filter == NULL || topic == NULL) {
		return false;
	}

	int i = 0;
	int j = 0;

	while (filter[i] != '\0' && topic[j] != '\0') {
		if (filter[i] == '#') {
			return true;
		}
		if (filter[i] == '+') {
			while (filter[i] != '/') {
				if (filter[i] == '\0') {
					return false;
				}
				i++;
			}
			while (topic[j] != '\0' && topic[j] != '/') {
				j++;
			}
		}

		if (filter[i] != topic[j]) {
			return false;
		}

		i += 1;
		j += 1;
	}

	return is_filtering_done(&filter[i], &topic[j]);
}

static unsigned int count_subscribers(topic_t *topic)
{
	return (unsigned int)topic->nr_subscriptions;
}

static size_t copy_subscriptions(const subscribe_t **new_subs,
		const subscribe_t **old_subs, size_t n)
{
	size_t count = 0;
	for (size_t i = 0; i < n; i++) {
		if (old_subs[i] != NULL) {
			new_subs[count++] = old_subs[i];
		}
	}
	return count;
}

static bool expand_subscription_capacity(topic_t *topic)
{
	uint8_t capacity = topic->max_subscriptions;
	uint8_t new_capacity = (uint8_t)(capacity * 2U);
	assert((uint16_t)capacity * 2U < (1U << 8));

	const subscribe_t **new_subs = (const subscribe_t **)
		calloc(new_capacity, sizeof(subscribe_t *));
	if (new_subs == NULL) {
		return false;
	}

	const subscribe_t **old_subs = topic->subscriptions;
	uint8_t count = (uint8_t)copy_subscriptions(new_subs, old_subs,
			(size_t)capacity);
	assert(count < new_capacity);
	assert(count == topic->nr_subscriptions);

	topic->max_subscriptions = new_capacity;
	topic->subscriptions = new_subs;
	free(old_subs);

	debug("Expanded from %u to %u", capacity, new_capacity);
	return true;
}

static void shrink_subscription_capacity(topic_t *topic)
{
	uint8_t capacity = topic->max_subscriptions;
	uint8_t new_capacity = capacity / 2U;

	if (capacity <= PUBSUB_MIN_SUBSCRIPTION_CAPACITY) {
		return;
	}
	if (count_subscribers(topic) * 2U >= capacity) {
		return;
	}

	const subscribe_t **new_subs = (const subscribe_t **)
		calloc(new_capacity, sizeof(subscribe_t *));
	if (new_subs == NULL) {
		return;
	}

	const subscribe_t **old_subs = topic->subscriptions;
	uint8_t count = (uint8_t)copy_subscriptions(new_subs, old_subs,
			(size_t)capacity);
	assert(count < new_capacity);
	assert(count == topic->nr_subscriptions);

	topic->max_subscriptions = new_capacity;
	topic->subscriptions = new_subs;
	free(old_subs);

	debug("Shrunken from %u to %u", capacity, new_capacity);
}

static void add_subscription(topic_t *topic, subscribe_t *sub)
{
	if (topic->nr_subscriptions >= topic->max_subscriptions) {
		if (!expand_subscription_capacity(topic)) {
			error("can't expand");
			return;
		}
	}

	for (uint8_t i = 0; i < topic->max_subscriptions; i++) {
		if (topic->subscriptions[i] == NULL) {
			topic->subscriptions[i] = sub;
			topic->nr_subscriptions++;
			debug("\"%p\" added in %s", sub, topic->name);
			return;
		}
	}

	error("unreachable");
}

static uint8_t subscribe_topics(subscribe_t *sub, const char *topic_filter)
{
	uint8_t count = 0;
	struct list *i;
	list_for_each(i, &m.pubsub_list) {
		topic_t *topic = list_entry(i, topic_t, pubsub_node);
		if (is_topic_matched_with(topic_filter, topic->name)) {
			add_subscription(topic, sub);
			count++;
		}
	}
	return count;
}

static void remove_subscription(topic_t *topic, subscribe_t *sub)
{
	for (uint8_t i = 0; i < topic->max_subscriptions; i++) {
		if (topic->subscriptions[i] == sub) {
			topic->subscriptions[i] = NULL;
			topic->nr_subscriptions--;
			debug("\"%p\" removed from %s", sub, topic->name);
			return;
		}
	}

	warn("\"%p\" not found in %s", sub, topic->name);
}

static void remove_subscriptions(topic_t *topic)
{
	for (uint8_t i = 0; i < topic->max_subscriptions; i++) {
		const subscribe_t *sub = topic->subscriptions[i];
		if (sub == NULL) {
			continue;
		}

		topic->subscriptions[i] = NULL;
		topic->nr_subscriptions--;

		if (!IS_SUBSCRIBER_STATIC(sub)) {
			intptr_t *p = (intptr_t *)&sub;
			free((void *)*p);
		}
	}

	assert(topic->nr_subscriptions == 0);
}

static void unsubscribe_internal(subscribe_t *sub)
{
	struct list *i;
	list_for_each(i, &m.pubsub_list) {
		topic_t *topic = list_entry(i, topic_t, pubsub_node);
		if (!is_topic_matched_with(sub->topic_filter, topic->name)) {
			continue;
		}
		info("Unsubscribe from %s", topic->name);
		remove_subscription(topic, sub);
		shrink_subscription_capacity(topic);
	}
}

static void publish_internal(const topic_t *topic,
		const void *msg, size_t msglen)
{
	for (uint8_t i = 0; i < topic->max_subscriptions; i++) {
		const subscribe_t *sub = topic->subscriptions[i];
		if (sub == NULL) {
			continue;
		}
		sub->callback(GET_SUBSCRIBER_CONTEXT(sub), msg, msglen);
	}
}

static void destroy_topic(topic_t *topic)
{
	remove_topic(topic);
	publish_internal(topic, PUBSUB_TOPIC_DESTROY_MESSAGE,
			sizeof(PUBSUB_TOPIC_DESTROY_MESSAGE));
	remove_subscriptions(topic);
}

static void pubsub_lock(void)
{
	pthread_mutex_lock(&m.pubsub_list_lock);
}

static void pubsub_unlock(void)
{
	pthread_mutex_unlock(&m.pubsub_list_lock);
}

static bool pubsub_ready(void)
{
	return m.initialized;
}

static void pubsub_init(void)
{
	list_init(&m.pubsub_list);
	pthread_mutex_init(&m.pubsub_list_lock, NULL);
	m.initialized = true;
}

static subscribe_t *subscribe_core(subscribe_t *sub, const char *topic_filter,
		pubsub_callback_t cb, void *context)
{
	if ((topic_filter == NULL) || (cb == NULL)) {
		return NULL;
	}

	uint8_t n = 0;

	pubsub_lock();
	{
		n = subscribe_topics(sub, topic_filter);
	}
	pubsub_unlock();

	if (n == 0) {
		return NULL;
	}

	sub->topic_filter = topic_filter;
	sub->callback = cb;
	sub->context = context;

	info("Subscribe to %s", topic_filter);

	return sub;
}

pubsub_error_t pubsub_create(const char *topic_name)
{
	if (!pubsub_ready()) {
		pubsub_init();
	}

	const topic_t *duplicated;
	topic_t *topic;
	pubsub_error_t err = PUBSUB_TOPIC_EXIST;
	size_t topic_len = (topic_name == NULL)? 0 :
		strnlen(topic_name, PUBSUB_TOPIC_NAME_MAXLEN);

	if (topic_len == 0) {
		return PUBSUB_INVALID_PARAM;
	}
	if (!(topic = (topic_t *)calloc(1, sizeof(*topic)))) {
		return PUBSUB_NO_MEMORY;
	}

	topic->nr_subscriptions = 0;
	topic->max_subscriptions = PUBSUB_MIN_SUBSCRIPTION_CAPACITY;
	if ((topic->subscriptions = (const subscribe_t **)
				calloc(topic->max_subscriptions,
					sizeof(subscribe_t *)))
			== NULL) {
		err = PUBSUB_NO_MEMORY;
		goto out_free_topic;
	}

	if (!(topic->name = (char *)calloc(1, topic_len + 1))) {
		err = PUBSUB_NO_MEMORY;
		goto out_free_subscription;
	}
	strncpy(topic->name, topic_name, topic_len);
	topic->name[topic_len] = '\0';

	pubsub_lock();
	{
		if ((duplicated = find_topic(topic_name)) == NULL) {
			add_topic(topic);
		}
	}
	pubsub_unlock();

	if (duplicated == NULL) {
		info("%s topic created", topic->name);
		return PUBSUB_SUCCESS;
	}

	free(topic->name);
out_free_subscription:
	free(topic->subscriptions);
out_free_topic:
	free(topic);
	return err;
}

pubsub_error_t pubsub_destroy(const char *topic_name)
{
	if (!topic_name) {
		return PUBSUB_INVALID_PARAM;
	}

	topic_t *topic;

	pubsub_lock();
	{
		if ((topic = find_topic(topic_name)) != NULL) {
			destroy_topic(topic);
		}
	}
	pubsub_unlock();

	if (topic == NULL) {
		return PUBSUB_TOPIC_NOT_EXIST;
	}

	info("%s " PUBSUB_TOPIC_DESTROY_MESSAGE, topic->name);

	free(topic->subscriptions);
	free(topic->name);
	free(topic);

	return PUBSUB_SUCCESS;
}

pubsub_error_t pubsub_publish(const char *topic_name,
		const void *msg, size_t msglen)
{
	const topic_t *topic;

	if (!topic_name || !msg || !msglen) {
		return PUBSUB_INVALID_PARAM;
	}

	pubsub_lock();
	{
		if ((topic = find_topic(topic_name)) != NULL) {
			publish_internal(topic, msg, msglen);
		}
	}
	pubsub_unlock();

	if (topic == NULL) {
		return PUBSUB_TOPIC_NOT_EXIST;
	}

	debug("Publish to %s", topic->name);

	return PUBSUB_SUCCESS;
}

/* NOTE: `topic_filter` must be kept even after registering the subscription
 * because we don't newly allocate memory for the topic filter but use its
 * pointer ever afterward. */
pubsub_subscribe_t *pubsub_subscribe_static(pubsub_subscribe_t *obj,
		const char *topic_filter, pubsub_callback_t cb, void *context)
{
	return (pubsub_subscribe_t *)
		subscribe_core((subscribe_t *)obj, topic_filter, cb,
				GET_CONTEXT_STATIC(context));
}

pubsub_subscribe_t *pubsub_subscribe(const char *topic_filter,
		pubsub_callback_t cb, void *context)
{
	subscribe_t *obj = (subscribe_t *)calloc(1, sizeof(*obj));

	if (obj == NULL) {
		return NULL;
	}

	if (subscribe_core(obj, topic_filter, cb, context) == NULL) {
		free(obj);
		return NULL;
	}

	return (pubsub_subscribe_t *)obj;
}

pubsub_error_t pubsub_unsubscribe(pubsub_subscribe_t *obj)
{
	subscribe_t *p = (subscribe_t *)obj;

	if (p == NULL || p->topic_filter == NULL) {
		return PUBSUB_INVALID_PARAM;
	}

	pubsub_lock();
	{
		unsubscribe_internal(p);
	}
	pubsub_unlock();

	if (!IS_SUBSCRIBER_STATIC(p)) {
		free(p);
	}

	return PUBSUB_SUCCESS;
}

int pubsub_count(const char *topic_name)
{
	topic_t *topic;
	int count = 0;

	if (topic_name == NULL) {
		return PUBSUB_INVALID_PARAM;
	}

	pubsub_lock();
	{
		if ((topic = find_topic(topic_name)) != NULL) {
			count = (int)count_subscribers(topic);
		}
	}
	pubsub_unlock();

	if (topic == NULL) {
		return PUBSUB_TOPIC_NOT_EXIST;
	}

	return count;
}

const char *pubsub_stringify_error(pubsub_error_t err)
{
	switch (err) {
	case PUBSUB_SUCCESS:
		return "success";
	case PUBSUB_ERROR:
		return "error";
	case PUBSUB_TOPIC_EXIST:
		return "topic already exist";
	case PUBSUB_TOPIC_NOT_EXIST:
		return "topic not exist";
	case PUBSUB_NO_MEMORY:
		return "no memory";
	case PUBSUB_INVALID_PARAM:
		return "invalid parameters";
	case PUBSUB_SUBSCRIBERS_EXIST:
		return "subscriber already exist";
	default:
		break;
	}
	return "unknown error";
}
