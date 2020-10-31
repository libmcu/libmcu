#ifndef APP_TIMER_SPY_H
#define APP_TIMER_SPY_H

#include <stdint.h>
#include "apptimer.h"

enum {
	TIMERSPY_CREATED		= 100,
	TIMERSPY_STARTED		= 1,
	TIMERSPY_STARTED_PERIODIC	= 2,
	TIMERSPY_STOPPED		= 3,
	TIMERSPY_COMPLETED		= 4,
	TIMERSPY_DELETED		= 5,
};

uint32_t apptimerspy_get_state(apptimer_t *timer);

#endif
