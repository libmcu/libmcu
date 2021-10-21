#ifndef LIBMCU_PUBSUB_H
#define LIBMCU_PUBSUB_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

#if !defined(PUBSUB_TOPIC_NAME_MAXLEN)
#define PUBSUB_TOPIC_NAME_MAXLEN		32
#endif

#if !defined(PUBSUB_DEBUG)
#define PUBSUB_DEBUG(...)
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
} pubsub_subscribe_static_t;

typedef pubsub_subscribe_static_t * pubsub_subscribe_t;
typedef void (*pubsub_callback_t)(void *context, const void *msg, size_t msglen);

/**
 * @brief Publish a message to a topic
 *
 * Calling all the callbacks registered in the subscriptions is done in the
 * context of the caller, which takes time to finish all. So a kind of task,
 * such a jobqueue, would help it run in another context.
 *
 * @param[in] topic is where the message gets publshed to
 * @param[in] msg A message to publish
 * @param[in] msglen The length of the message
 *
 * @return error code in @ref pubsub_error_t
 */
pubsub_error_t pubsub_publish(const char *topic, const void *msg, size_t msglen);

/**
 * @note `topic_filter` should be kept in valid memory space even after
 * registered. Because it keeps dereferencing the pointer of `topic_filter`
 * ever afterward until unsubscribing.
 */
pubsub_subscribe_t pubsub_subscribe_static(pubsub_subscribe_t handle,
		const char *topic_filter, pubsub_callback_t cb, void *context);
pubsub_subscribe_t pubsub_subscribe(const char *topic_filter,
		pubsub_callback_t cb, void *context);
pubsub_error_t pubsub_unsubscribe(pubsub_subscribe_t handle);

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
