#include <time.h>

#include "FreeRTOS.h"
#include "task.h"

time_t time(time_t *c_time)
{
	time_t ltime = (time_t)xTaskGetTickCount();

	if (c_time)
		*c_time = ltime;

	return ltime;
}
