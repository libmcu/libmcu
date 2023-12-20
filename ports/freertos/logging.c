#include "libmcu/logging_overrides.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <stdbool.h>

static unsigned long intctx;

static bool in_interrupt(void)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
	return xPortIsInsideInterrupt();
#pragma GCC diagnostic pop
}

void logging_lock_init(void)
{
	/* Platform specific implementation */
}

void logging_lock(void)
{
	if (in_interrupt()) {
		intctx = taskENTER_CRITICAL_FROM_ISR();
	} else {
		taskENTER_CRITICAL();
	}
}

void logging_unlock(void)
{
	if (in_interrupt()) {
		taskEXIT_CRITICAL_FROM_ISR(intctx);
	} else {
		taskEXIT_CRITICAL();
	}
}
