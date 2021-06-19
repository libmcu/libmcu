#include "libmcu/timext.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define time_after(goal, chasing)	((int)(goal)    - (int)(chasing) < 0)
#define time_before(goal, chasing)	((int)(chasing) - (int)(goal)    < 0)

void timeout_set(unsigned int *goal, unsigned int msec)
{
	*goal = xTaskGetTickCount() + pdMS_TO_TICKS(msec);
}

bool timeout_is_expired(unsigned int goal)
{
	return time_after(goal, xTaskGetTickCount());
}

void sleep_ms(unsigned int msec)
{
	vTaskDelay(pdMS_TO_TICKS(msec));
}
