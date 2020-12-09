#ifndef LIBMCU_PUBSUB_H
#define LIBMCU_PUBSUB_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

#define PUBSUB_TOPIC_NAME_MAXLEN		32

typedef enum {
	PUBSUB_SUCCESS				= 0,
	PUBSUB_ERROR				= -1,
	PUBSUB_TOPIC_EXIST			= -2,
	PUBSUB_TOPIC_NOT_EXIST			= -3,
	PUBSUB_NO_MEMORY			= -4,
	PUBSUB_INVALID_PARAM			= -5,
	PUBSUB_SUBSCRIBERS_EXIST		= -6,
} pubsub_error_t;

typedef union {
#if defined(__WORDSIZE) && __WORDSIZE == 32
	char _size[16];
#else // 64-bit
	char _size[32];
#endif
	long _align;
} pubsub_subscribe_t;

typedef void (*pubsub_callback_t)(void *context, const void *msg, size_t msglen);

pubsub_error_t pubsub_create(const char *topic_name);
pubsub_error_t pubsub_destroy(const char *topic_name);
/** Publish a message to a topic
 *
 * It delivers the message for all subscribers in the context of the caller.
 * So it takes time to finish calling all the callbacks registered in
 * subscriptions. You may want some kind of task to make it run in another
 * context, using such a jobpool.
 *
 * @param topic_name is where the message gets publshed to
 * @param msg A message to publish
 * @param msglen The length of the message
 */
pubsub_error_t pubsub_publish(const char *topic_name,
		const void *msg, size_t msglen);

pubsub_subscribe_t *pubsub_subscribe_static(pubsub_subscribe_t *obj,
		const char *topic_name, pubsub_callback_t cb, void *context);
pubsub_subscribe_t *pubsub_subscribe(const char *topic_name,
		pubsub_callback_t cb, void *context);
pubsub_error_t pubsub_unsubscribe(pubsub_subscribe_t *obj);

int pubsub_count(const char *topic_name);
const char *pubsub_stringify_error(pubsub_error_t err);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_PUBSUB_H */
