#ifndef LIBMCU_APPTIMER_H
#define LIBMCU_APPTIMER_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#define APPTIMER_MAX_TIMEOUT		\
	((1UL << (sizeof(apptimer_timeout_t) * CHAR_BIT - 1)) - 1)

typedef enum {
	APPTIMER_SUCCESS		= 0,
	APPTIMER_ERROR			= -1,
	APPTIMER_INVALID_PARAM		= -2,
	APPTIMER_ALREADY_STARTED	= -3,
	APPTIMER_TIME_LIMIT_EXCEEDED	= -4,
} apptimer_error_t;

typedef union {
#if defined(__WORDSIZE) && __WORDSIZE == 64
	char _size[56];
#else
	char _size[28];
#endif
	long _align;
} apptimer_t;

typedef void (*apptimer_callback_t)(void *context);
typedef uintptr_t apptimer_timeout_t;

/** Initialize apptimer
 *
 * @param update_alarm typically sets hardware timer counter to get notified at
 * the timeout expiration.
 */
void apptimer_init(void (*update_alarm)(apptimer_timeout_t timeout));
apptimer_error_t apptimer_deinit(void);

apptimer_t *apptimer_create(bool repeat, apptimer_callback_t callback);
apptimer_t *apptimer_create_static(apptimer_t * const timer, bool repeat,
		apptimer_callback_t callback);
apptimer_error_t apptimer_delete(apptimer_t *timer);

apptimer_error_t apptimer_start(apptimer_t * const timer,
		apptimer_timeout_t timeout, void *callback_context);
apptimer_error_t apptimer_stop(apptimer_t * const timer);
int apptimer_count(void);

/** Process expirations and bookkeepings
 *
 * It should not be called from the interrupt context as expiry processing gets
 * done here.
 */
void apptimer_schedule(apptimer_timeout_t time_elapsed);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_APPTIMER_H */
