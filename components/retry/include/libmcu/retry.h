#ifndef LIBMCU_RETRY_H
#define LIBMCU_RETRY_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(RETRY_DEBUG)
#define RETRY_DEBUG(...)
#endif

#include <stdint.h>

typedef enum {
	RETRY_SUCCESS		= 0,
	RETRY_EXHAUSTED,
} retry_error_t;

typedef struct {
	uint32_t max_backoff_ms;
	uint16_t min_backoff_ms;
	uint16_t max_jitter_ms;
	uint16_t max_attempts;
	uint16_t attempts;
	uint32_t previous_backoff_ms;
	void (*sleep)(unsigned int msec);
} retry_t;

/** Backoff for an amount of time
 *
 * The sleep callback must be set first before calling `retry_backoff()`. The
 * default parameters are taken if the argument is zero initialized. Don't
 * forget that a stack variable has gabage value before initializing.
 *
 * Below is the minimal usage:
 *
 * 	retry_t retry = { .sleep = sleep_ms, };
 * 	do {
 * 		if (do_something() == true) {
 * 			break;
 * 		}
 * 	} while (retry_backoff(&retry) != RETRY_EXHAUSTED);
 */
retry_error_t retry_backoff(retry_t *param);
void retry_reset(retry_t *param);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RETRY_H */
