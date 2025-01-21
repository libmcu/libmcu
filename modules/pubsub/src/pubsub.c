/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/pubsub.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>

#include "libmcu/compiler.h"
#include "libmcu/assert.h"

#if !defined(PUBSUB_MIN_SUBSCRIPTION_CAPACITY)
#define PUBSUB_MIN_SUBSCRIPTION_CAPACITY		4
#endif

/* NOTE: It sets the least significant bit of `subscriber->context` to
 * differentiate static subscriber from one created dynamically. */
#define GET_SUBSCRIBER_CONTEXT(handle)			\
	(void *)((uintptr_t)(handle)->context & ~1UL)
#define IS_SUBSCRIBER_STATIC(handle)			\
	((uintptr_t)(handle)->context & 1UL)
#define GET_CONTEXT_STATIC(ctx)				\
	(void *)((uintptr_t)(ctx) | 1UL)

struct subscription {
	const char *topic_filter;
	pubsub_callback_t callback;
	void *context;
	intptr_t _placeholder_for_compatibility_to_tiny_pubsub;
};
static_assert(sizeof(struct subscription)
		== sizeof(pubsub_subscribe_static_t), "");

static struct {
	struct {
		pthread_mutex_t lock;
		const struct subscription **pool; // dynamically allocated array
		uint8_t capacity;
		uint8_t length;
	} subscription;
} m;

static void get_next_topic_word(const char **s)
{
	while (**s != '/' && **s != '\0') {
		(*s)++;
	}
}

static bool is_topic_matched_with(const char *filter, const char *topic)
{
	while (*filter != '\0' && *topic != '\0') {
		if (*filter == '#') {
			return true;
		}
		if (*filter == '+') {
			get_next_topic_word(&filter);
			get_next_topic_word(&topic);
			continue;
		}

		if (*filter != *topic) {
			return false;
		}

		filter++;
		topic++;
	}

	if (*filter != '\0') {
		return false;
	}
	if (*topic == '\0') {
		return true;
	}

	return false;
}

static unsigned int count_subscribers(const char *topic)
{
	uint8_t count = 0;

	for (uint8_t i = 0; i < m.subscription.capacity; i++) {
		const struct subscription *sub = m.subscription.pool[i];
		if (sub != NULL &&
				is_topic_matched_with(sub->topic_filter, topic)) {
			count++;
		}
	}

	return (unsigned int)count;
}

static size_t copy_subscriptions(const struct subscription **new_subs,
		const struct subscription **old_subs, size_t n)
{
	size_t count = 0;

	for (size_t i = 0; i < n; i++) {
		if (old_subs[i] != NULL) {
			new_subs[count++] = old_subs[i];
		}
	}

	return count;
}

static bool expand_subscription_capacity(void)
{
	uint8_t capacity = m.subscription.capacity;
	uint8_t new_capacity = (uint8_t)(capacity * 2U);
	assert((uint16_t)capacity * 2U < (1U << 8));

	const struct subscription **new_subs = (const struct subscription **)
		calloc(new_capacity, sizeof(struct subscription *));
	if (new_subs == NULL) {
		return false;
	}

	const struct subscription **old_subs = m.subscription.pool;
	uint8_t count = (uint8_t)copy_subscriptions(new_subs, old_subs,
			(size_t)capacity);
	assert(count < new_capacity);
	assert(count == m.subscription.length);

	m.subscription.capacity = new_capacity;
	m.subscription.pool = new_subs;
	free(old_subs);

	PUBSUB_DEBUG("Expanded from %u to %u", capacity, new_capacity);
	return true;
}

static void shrink_subscription_capacity(void)
{
	uint8_t capacity = m.subscription.capacity;
	uint8_t new_capacity = capacity / 2U;

	if (capacity <= PUBSUB_MIN_SUBSCRIPTION_CAPACITY) {
		return;
	}
	if (m.subscription.length * 2U >= capacity) {
		return;
	}

	const struct subscription **new_subs = (const struct subscription **)
		calloc(new_capacity, sizeof(struct subscription *));
	if (new_subs == NULL) {
		return;
	}

	const struct subscription **old_subs = m.subscription.pool;
	uint8_t count = (uint8_t)copy_subscriptions(new_subs, old_subs,
			(size_t)capacity);
	assert(count < new_capacity);
	assert(count == m.subscription.length);

	m.subscription.capacity = new_capacity;
	m.subscription.pool = new_subs;
	free(old_subs);

	PUBSUB_DEBUG("Shrunken from %u to %u", capacity, new_capacity);
}

static bool register_subscription(const struct subscription *sub)
{
	if (m.subscription.length >= m.subscription.capacity) {
		if (!expand_subscription_capacity()) {
			PUBSUB_DEBUG("can't expand");
			return false;
		}
	}

	for (uint8_t i = 0; i < m.subscription.capacity; i++) {
		if (m.subscription.pool[i] == NULL) {
			m.subscription.pool[i] = sub;
			m.subscription.length++;
			PUBSUB_DEBUG("%p added for \"%s\"",
					sub, sub->topic_filter);
			return true;
		}
	}

	assert(0); // can not reach down here
	return false;
}

static bool unregister_subscription(const struct subscription *sub)
{
	for (uint8_t i = 0; i < m.subscription.capacity; i++) {
		if (m.subscription.pool[i] == sub) {
			m.subscription.pool[i] = NULL;
			m.subscription.length--;
			shrink_subscription_capacity();
			PUBSUB_DEBUG("%p removed from \"%s\"",
					sub, sub->topic_filter);
			return true;
		}
	}

	PUBSUB_DEBUG("%p for \"%s\" not found", sub, sub->topic_filter);
	return false;
}

static void publish_internal(const char *topic, const void *msg, size_t msglen)
{
	for (uint8_t i = 0; i < m.subscription.capacity; i++) {
		const struct subscription *sub = m.subscription.pool[i];
		if (sub == NULL) {
			continue;
		}
		if (is_topic_matched_with(sub->topic_filter, topic)) {
			sub->callback(GET_SUBSCRIBER_CONTEXT(sub), msg, msglen);
		}
	}
}

static void subscriptions_lock(void)
{
	pthread_mutex_lock(&m.subscription.lock);
}

static void subscriptions_unlock(void)
{
	pthread_mutex_unlock(&m.subscription.lock);
}

static struct subscription *subscribe_core(struct subscription *sub,
		const char *topic_filter, pubsub_callback_t cb, void *context)
{
	bool result = false;

	if ((topic_filter == NULL) || (cb == NULL)) {
		return NULL;
	}

	sub->topic_filter = topic_filter;
	sub->callback = cb;
	sub->context = context;

	subscriptions_lock();
	{
		result = register_subscription(sub);
	}
	subscriptions_unlock();

	if (!result) {
		return NULL;
	}

	PUBSUB_DEBUG("Subscribe to \"%s\"", topic_filter);

	return sub;
}

pubsub_error_t pubsub_create(const char *topic)
{
	unused(topic);
	return PUBSUB_SUCCESS;
}

pubsub_error_t pubsub_destroy(const char *topic)
{
	unused(topic);
	return PUBSUB_SUCCESS;
}

pubsub_error_t pubsub_publish(const char *topic, const void *msg, size_t msglen)
{
	if ((topic == NULL) || (msg == NULL && msglen > 0)) {
		return PUBSUB_INVALID_PARAM;
	}

	subscriptions_lock();
	{
		publish_internal(topic, msg, msglen);
	}
	subscriptions_unlock();

	return PUBSUB_SUCCESS;
}

pubsub_subscribe_t pubsub_subscribe_static(pubsub_subscribe_t handle,
		const char *topic_filter, pubsub_callback_t cb, void *context)
{
	return (pubsub_subscribe_t)
		subscribe_core((struct subscription *)handle, topic_filter, cb,
				GET_CONTEXT_STATIC(context));
}

pubsub_subscribe_t pubsub_subscribe(const char *topic_filter,
		pubsub_callback_t cb, void *context)
{
	struct subscription *handle = (struct subscription *)
		calloc(1, sizeof(*handle));

	if (handle == NULL) {
		return NULL;
	}

	if (subscribe_core(handle, topic_filter, cb, context) == NULL) {
		free(handle);
		return NULL;
	}

	return (pubsub_subscribe_t)handle;
}

pubsub_error_t pubsub_unsubscribe(pubsub_subscribe_t handle)
{
	struct subscription *sub = (struct subscription *)handle;
	bool result = false;

	if (sub == NULL || sub->topic_filter == NULL) {
		return PUBSUB_INVALID_PARAM;
	}

	subscriptions_lock();
	{
		result = unregister_subscription(sub);
	}
	subscriptions_unlock();

	if (!result) {
		return PUBSUB_NO_EXIST_SUBSCRIBER;
	}

	PUBSUB_DEBUG("Unsubscribe from \"%s\"", sub->topic_filter);

	if (!IS_SUBSCRIBER_STATIC(sub)) {
		free(sub);
	}

	return PUBSUB_SUCCESS;
}

int pubsub_count(const char *topic)
{
	int count = 0;

	if (topic == NULL) {
		return PUBSUB_INVALID_PARAM;
	}

	subscriptions_lock();
	{
		count = (int)count_subscribers(topic);
	}
	subscriptions_unlock();

	return count;
}

void pubsub_init(void)
{
	pthread_mutex_init(&m.subscription.lock, NULL);

	m.subscription.length = 0;
	m.subscription.capacity = PUBSUB_MIN_SUBSCRIPTION_CAPACITY;
	m.subscription.pool = (const struct subscription **)
		calloc(m.subscription.capacity, sizeof(struct subscription *));

	assert(m.subscription.pool != NULL);
}

void pubsub_deinit(void)
{
	subscriptions_lock();

	for (uint8_t i = 0; i < m.subscription.capacity; i++) {
		const struct subscription *sub = m.subscription.pool[i];
		if (sub == NULL) {
			continue;
		}
		if (!IS_SUBSCRIBER_STATIC(sub)) {
			intptr_t *p = (intptr_t *)&sub;
			free((void *)*p);
		}
	}

	free(m.subscription.pool);
}

const char *pubsub_stringify_error(pubsub_error_t err)
{
	switch (err) {
	case PUBSUB_SUCCESS:
		return "success";
	case PUBSUB_ERROR:
		return "error";
	case PUBSUB_EXIST_TOPIC:
		return "exist topic";
	case PUBSUB_NO_EXIST_TOPIC:
		return "no exist topic";
	case PUBSUB_NO_MEMORY:
		return "no memory";
	case PUBSUB_INVALID_PARAM:
		return "invalid parameters";
	case PUBSUB_EXIST_SUBSCRIBER:
		return "exist subscriber";
	case PUBSUB_NO_EXIST_SUBSCRIBER:
		return "no exist subscriber";
	default:
		break;
	}
	return "unknown error";
}
