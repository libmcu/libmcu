#include "libmcu/metrics_overrides.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

static unsigned long intctx;

static bool in_interrupt(void)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
	return xPortIsInsideInterrupt();
#pragma GCC diagnostic pop
}

void metrics_lock_init(void)
{
	/* Platform specific implementation */
}

void metrics_lock(void)
{
	if (in_interrupt()) {
		intctx = taskENTER_CRITICAL_FROM_ISR();
	} else {
		taskENTER_CRITICAL();
	}
}

void metrics_unlock(void)
{
	if (in_interrupt()) {
		taskEXIT_CRITICAL_FROM_ISR(intctx);
	} else {
		taskEXIT_CRITICAL();
	}
}
