#include "FreeRTOS.h"
#include <stdint.h>

BaseType_t xPortIsInsideInterrupt(void)
{
	uint32_t is_in_intr;

	__asm__ __volatile__("mrs %0, ipsr" : "=r"(is_in_intr) :: "memory" );

	if (is_in_intr) {
		return pdTRUE;
	}

	return pdFALSE;
}
