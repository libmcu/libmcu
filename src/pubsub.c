#include "libmcu/pubsub.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include "libmcu/list.h"
#include "logger.h"

#if !defined(PUBSUB_TOPIC_DESTROY_MESSAGE)
#define PUBSUB_TOPIC_DESTROY_MESSAGE		"topic destroyed"
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
	struct list subscriptions; // list head for subscriptions
} topic_t;

typedef struct {
	topic_t *topic;
	struct list subscription_node;
	pubsub_callback_t callback;
	void *context;
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

static void initialize_subscriptions(topic_t *topic)
{
	list_init(&topic->subscriptions);
}

static void remove_subscriptions(topic_t *topic)
{
	struct list *i, *j;
	list_for_each_safe(i, j, &topic->subscriptions) {
		subscribe_t *sub =
			list_entry(i, subscribe_t, subscription_node);
		list_del(&sub->subscription_node, &topic->subscriptions);
		if (!IS_SUBSCRIBER_STATIC(sub)) {
			free(sub);
		}
	}
}

static int count_subscribers(topic_t *topic)
{
	int count = 0;

	struct list *i;
	list_for_each(i, &topic->subscriptions) {
		count++;
	}

	return count;
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

static void publish_internal(const topic_t *topic,
		const void *msg, size_t msglen)
{
	struct list *i;
	list_for_each(i, &topic->subscriptions) {
		subscribe_t *sub =
			list_entry(i, subscribe_t, subscription_node);
		sub->callback(GET_SUBSCRIBER_CONTEXT(sub), msg, msglen);
	}
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

static subscribe_t *subscribe_core(subscribe_t *obj,
		const char *topic_name, pubsub_callback_t cb, void *context)
{
	topic_t *topic;

	if ((topic_name == NULL) || (cb == NULL)) {
		return NULL;
	}

	pubsub_lock();
	{
		if ((topic = find_topic(topic_name)) != NULL) {
			obj->topic = topic;
			list_add(&obj->subscription_node, &topic->subscriptions);
		}
	}
	pubsub_unlock();

	if (topic == NULL) {
		return NULL;
	}

	obj->callback = cb;
	obj->context = context;

	debug("Subscribe to %s", topic_name);

	return obj;
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
	if (!(topic->name = (char *)calloc(1, topic_len + 1))) {
		err = PUBSUB_NO_MEMORY;
		goto out_free;
	}
	strncpy(topic->name, topic_name, topic_len);
	topic->name[topic_len] = '\0';
	initialize_subscriptions(topic);

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
out_free:
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
			remove_topic(topic);

			publish_internal(topic, PUBSUB_TOPIC_DESTROY_MESSAGE,
					sizeof(PUBSUB_TOPIC_DESTROY_MESSAGE));
			remove_subscriptions(topic);
		}
	}
	pubsub_unlock();

	if (topic == NULL) {
		return PUBSUB_TOPIC_NOT_EXIST;
	}

	info("%s " PUBSUB_TOPIC_DESTROY_MESSAGE, topic->name);

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

pubsub_subscribe_t *pubsub_subscribe_static(pubsub_subscribe_t *obj,
		const char *topic_name, pubsub_callback_t cb, void *context)
{
	return (pubsub_subscribe_t *)
		subscribe_core((subscribe_t *)obj, topic_name, cb,
				GET_CONTEXT_STATIC(context));
}

pubsub_subscribe_t *pubsub_subscribe(const char *topic_name,
		pubsub_callback_t cb, void *context)
{
	subscribe_t *obj = (subscribe_t *)calloc(1, sizeof(*obj));

	if (obj == NULL) {
		return NULL;
	}

	if (subscribe_core(obj, topic_name, cb, context) == NULL) {
		free(obj);
		return NULL;
	}

	return (pubsub_subscribe_t *)obj;
}

pubsub_error_t pubsub_unsubscribe(pubsub_subscribe_t *obj)
{
	subscribe_t *p = (subscribe_t *)obj;

	if (!p || !p->topic) {
		return PUBSUB_INVALID_PARAM;
	}

	debug("Unsubscribe from %s", p->topic->name);

	pubsub_lock();
	{
		list_del(&p->subscription_node, &p->topic->subscriptions);
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
			count = count_subscribers(topic);
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
