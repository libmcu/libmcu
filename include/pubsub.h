#ifndef PUBSUB_H
#define PUBSUB_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

#define PUBSUB_TOPIC_NAME_MAXLEN		16

typedef enum {
	PUBSUB_SUCCESS				= 0,
	PUBSUB_ERROR				= -1,
	PUBSUB_TOPIC_EXIST			= -2,
	PUBSUB_TOPIC_NOT_EXIST			= -3,
	PUBSUB_NO_MEMORY			= -4,
	PUBSUB_INVALID_PARAM			= -5,
	PUBSUB_SUBSCRIBERS_EXIST		= -6,
} pubsub_error_t;

typedef void (*pubsub_callback_t)(void *context, const void *msg, size_t msglen);
typedef struct pubsub_subscribe pubsub_subscribe_t;

pubsub_error_t pubsub_create(const char * const topic);
pubsub_error_t pubsub_destroy(const char * const topic);
/** Publish a message to a topic
 *
 * It delivers the message for all subscribers in the context of the caller.
 * So the caller of publish takes time to finish calling all the callbacks
 * registered in subscriptions. You may want some kind of signal to make it to
 * run in another context such as jobpool.
 *
 * @param topic is where the message gets publshed to
 * @param msg A message to publish
 * @param msglen The length of the message
 */
pubsub_error_t pubsub_publish(const char * const topic,
		const void * const msg, size_t msglen);
pubsub_subscribe_t *pubsub_subscribe(const char * const topic,
		pubsub_callback_t cb, void *context);
pubsub_error_t pubsub_unsubscribe(pubsub_subscribe_t *sub);
int pubsub_count(const char * const topic);
const char *pubsub_stringify_error(pubsub_error_t err);

#if defined(__cplusplus)
}
#endif

#endif /* PUBSUB_H */
