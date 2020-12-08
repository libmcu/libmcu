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

typedef struct {
	char *name;
	struct list pubsub_node; // list entry for the pubsub_list
	struct list subscriptions; // list head for subscriptions
} topic_t;

struct pubsub_subscribe_s {
	topic_t *topic;
	struct list subscription_node;
	pubsub_callback_t callback;
	void *context;
};

static struct {
	struct list pubsub_list;
	pthread_mutex_t pubsub_list_lock;
	volatile bool initialized;
} m;

static inline void add_topic(topic_t *topic)
{
	list_add(&topic->pubsub_node, &m.pubsub_list);
}

static inline void remove_topic(topic_t *topic)
{
	list_del(&topic->pubsub_node, &m.pubsub_list);
}

static inline void initialize_subscriptions(topic_t *topic)
{
	list_init(&topic->subscriptions);
}

static inline void remove_subscriptions(topic_t *topic)
{
	struct list *i, *j;
	list_for_each_safe(i, j, &topic->subscriptions) {
		pubsub_subscribe_t *sub = list_entry(i,
				pubsub_subscribe_t, subscription_node);
		list_del(&sub->subscription_node, &topic->subscriptions);
		free(sub);
	}
}

static inline int count_subscribers(topic_t *topic)
{
	int count = 0;
	struct list *i;
	list_for_each(i, &topic->subscriptions) {
		count++;
	}
	return count;
}

static topic_t *find_topic(const char * const topic_name)
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

static void publish_internal(const topic_t * const topic,
		const void * const msg, size_t msglen)
{
	struct list *i;
	list_for_each(i, &topic->subscriptions) {
		pubsub_subscribe_t *sub = list_entry(i,
				pubsub_subscribe_t, subscription_node);
		sub->callback(sub->context, msg, msglen);
	}
}

pubsub_error_t pubsub_create(const char * const topic_name)
{
	pubsub_error_t err = PUBSUB_TOPIC_EXIST;
	topic_t *new_topic;
	size_t topic_len;

	if (!m.initialized) {
		list_init(&m.pubsub_list);
		pthread_mutex_init(&m.pubsub_list_lock, NULL);
		m.initialized = true;
	}

	if (topic_name == NULL) {
		return PUBSUB_INVALID_PARAM;
	}
	topic_len = strnlen(topic_name, PUBSUB_TOPIC_NAME_MAXLEN);

	if (!(new_topic = (topic_t *)calloc(1, sizeof(*new_topic)))) {
		return PUBSUB_NO_MEMORY;
	}
	if (!(new_topic->name = (char *)calloc(1, topic_len + 1))) {
		err = PUBSUB_NO_MEMORY;
		goto out_free;
	}
	strncpy(new_topic->name, topic_name, topic_len);
	new_topic->name[topic_len] = '\0';
	initialize_subscriptions(new_topic);

	const topic_t *p;

	pthread_mutex_lock(&m.pubsub_list_lock);
	{
		if ((p = find_topic(topic_name)) == NULL) {
			add_topic(new_topic);
		}
	}
	pthread_mutex_unlock(&m.pubsub_list_lock);

	if (p == NULL) {
		debug("%s topic created", new_topic->name);
		return PUBSUB_SUCCESS;
	}

	free(new_topic->name);
out_free:
	free(new_topic);
	return err;
}

pubsub_error_t pubsub_destroy(const char * const topic_name)
{
	if (!topic_name) {
		return PUBSUB_INVALID_PARAM;
	}

	topic_t *topic;

	pthread_mutex_lock(&m.pubsub_list_lock);
	{
		if ((topic = find_topic(topic_name)) != NULL) {
			remove_topic(topic);

			publish_internal(topic, PUBSUB_TOPIC_DESTROY_MESSAGE,
					sizeof(PUBSUB_TOPIC_DESTROY_MESSAGE));
			remove_subscriptions(topic);
		}
	}
	pthread_mutex_unlock(&m.pubsub_list_lock);

	if (topic == NULL) {
		return PUBSUB_TOPIC_NOT_EXIST;
	}

	debug("%s " PUBSUB_TOPIC_DESTROY_MESSAGE, topic->name);

	free(topic->name);
	free(topic);

	return PUBSUB_SUCCESS;
}

pubsub_subscribe_t *pubsub_subscribe(const char * const topic_name,
		pubsub_callback_t cb, void *context)
{
	pubsub_subscribe_t *sub;
	topic_t *topic;

	if ((topic_name == NULL) || (cb == NULL)) {
		return NULL;
	}
	if ((sub = (pubsub_subscribe_t *)calloc(1, sizeof(*sub))) == NULL) {
		return NULL;
	}

	sub->callback = cb;
	sub->context = context;

	pthread_mutex_lock(&m.pubsub_list_lock);
	{
		if ((topic = find_topic(topic_name)) != NULL) {
			sub->topic = topic;
			list_add(&sub->subscription_node, &topic->subscriptions);
		}
	}
	pthread_mutex_unlock(&m.pubsub_list_lock);

	if (topic == NULL) {
		free(sub);
		return NULL;
	}

	return sub;
}

pubsub_error_t pubsub_unsubscribe(pubsub_subscribe_t *sub)
{
	if (!sub || !sub->topic) {
		return PUBSUB_INVALID_PARAM;
	}

	pthread_mutex_lock(&m.pubsub_list_lock);
	{
		list_del(&sub->subscription_node, &sub->topic->subscriptions);
	}
	pthread_mutex_unlock(&m.pubsub_list_lock);

	free(sub);

	return PUBSUB_SUCCESS;
}

pubsub_error_t pubsub_publish(const char * const topic_name,
		const void * const msg, size_t msglen)
{
	const topic_t *topic;

	if (!topic_name || !msg || !msglen) {
		return PUBSUB_INVALID_PARAM;
	}

	pthread_mutex_lock(&m.pubsub_list_lock);
	{
		if ((topic = find_topic(topic_name)) != NULL) {
			publish_internal(topic, msg, msglen);
		}
	}
	pthread_mutex_unlock(&m.pubsub_list_lock);

	if (topic == NULL) {
		return PUBSUB_TOPIC_NOT_EXIST;
	}

	return PUBSUB_SUCCESS;
}

int pubsub_count(const char * const topic_name)
{
	topic_t *topic;
	int count = 0;

	if (!topic_name) {
		return PUBSUB_INVALID_PARAM;
	}

	pthread_mutex_lock(&m.pubsub_list_lock);
	{
		if ((topic = find_topic(topic_name)) != NULL) {
			count = count_subscribers(topic);
		}
	}
	pthread_mutex_unlock(&m.pubsub_list_lock);

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
