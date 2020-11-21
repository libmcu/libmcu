#include "libmcu/sleep.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void sleep_ms(unsigned long msec)
{
	vTaskDelay(msec / portTICK_PERIOD_MS);
}
