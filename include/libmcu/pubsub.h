#ifndef LIBMCU_PUBSUB_H
#define LIBMCU_PUBSUB_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

#if !defined(PUBSUB_TOPIC_NAME_MAXLEN)
#define PUBSUB_TOPIC_NAME_MAXLEN		32
#endif

typedef enum {
	PUBSUB_SUCCESS				= 0,
	PUBSUB_ERROR				= -1,
	PUBSUB_EXIST_TOPIC			= -2,
	PUBSUB_NO_EXIST_TOPIC			= -3,
	PUBSUB_NO_MEMORY			= -4,
	PUBSUB_INVALID_PARAM			= -5,
	PUBSUB_EXIST_SUBSCRIBER			= -6,
	PUBSUB_NO_EXIST_SUBSCRIBER		= -7,
} pubsub_error_t;

typedef union {
#if defined(__amd64__) || defined(__x86_64__) || defined(__aarch64__) \
	|| defined(__ia64__) || defined(__ppc64__)
	char _size[32];
#else // 32-bit
	char _size[16];
#endif
	long _align;
} pubsub_subscribe_t;

typedef void (*pubsub_callback_t)(void *context, const void *msg, size_t msglen);

/** Publish a message to a topic
 *
 * It delivers the message for all subscribers in the context of the caller.
 * So it takes time to finish calling all the callbacks registered in
 * subscriptions. You may want some kind of task to make it run in another
 * context, using such a jobqueue.
 *
 * @param topic is where the message gets publshed to
 * @param msg A message to publish
 * @param msglen The length of the message
 */
pubsub_error_t pubsub_publish(const char *topic, const void *msg, size_t msglen);

/* NOTE: `topic_filter` must be kept even after registering the subscription
 * because we don't newly allocate memory for the topic filter but use its
 * pointer ever afterward. */
pubsub_subscribe_t *pubsub_subscribe_static(pubsub_subscribe_t *obj,
		const char *topic_filter, pubsub_callback_t cb, void *context);
pubsub_subscribe_t *pubsub_subscribe(const char *topic_filter,
		pubsub_callback_t cb, void *context);
pubsub_error_t pubsub_unsubscribe(pubsub_subscribe_t *obj);

int pubsub_count(const char *topic);

pubsub_error_t pubsub_create(const char *topic);
pubsub_error_t pubsub_destroy(const char *topic);

const char *pubsub_stringify_error(pubsub_error_t err);

void pubsub_init(void);
void pubsub_deinit(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_PUBSUB_H */
