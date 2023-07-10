#include "libmcu/compiler.h"
#include "libmcu/logging.h"
#include "libmcu/assert.h"

#include "FreeRTOS.h"
#include "task.h"

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
extern void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
LIBMCU_NO_INSTRUMENT
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	unused(xTask);
	error("Stack overflow! %s", pcTaskName);
	assert(0);
}
#endif
