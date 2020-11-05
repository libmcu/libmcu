#ifndef APPTIMER_H
#define APPTIMER_H 1

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	APPTIMER_SUCCESS		= 0,
	APPTIMER_ERROR			= -1,
	APPTIMER_INVALID_PARAM		= -2,
} apptimer_error_t;

typedef union {
#if defined(__WORDSIZE) && __WORDSIZE == 64
	char _size[24];
#else
	char _size[16];
#endif
	long _align;
} apptimer_t;

int apptimer_init(void);
int apptimer_deinit(void);

apptimer_t *apptimer_create(uint32_t timeout_ms, bool repeat,
		void (*callback)(void *param));
int apptimer_create_static(apptimer_t *timer, uint32_t timeout_ms, bool repeat,
		void (*callback)(void *param));
int apptimer_delete(apptimer_t *timer);

int apptimer_start(apptimer_t *timer);
int apptimer_stop(apptimer_t *timer);

#if defined(__cplusplus)
}
#endif

#endif /* APPTIMER_H */
