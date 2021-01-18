#include "include/timext.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define time_after(goal, chasing)	((int)(goal)    - (int)(chasing) < 0)
#define time_before(goal, chasing)	((int)(chasing) - (int)(goal)    < 0)

#define MSEC_TO_TICKS(msec)		((msec) / portTICK_PERIOD_MS)
#define TICKS_TO_MSEC(ticks)		((ticks) * portTICK_PERIOD_MS)

unsigned int timeout_set(unsigned int msec)
{
	return xTaskGetTickCount() + MSEC_TO_TICKS(msec);
}

bool timeout_is_expired(unsigned int goal)
{
	return time_after(goal, xTaskGetTickCount());
}

void sleep_ms(unsigned int msec)
{
	vTaskDelay(MSEC_TO_TICKS(msec));
}
