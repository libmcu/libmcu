/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/board.h"
#include "libmcu/compiler.h"
#include "libmcu/assert.h"
#include "libmcu/logging.h"

#include "FreeRTOS.h"
#include "task.h"

LIBMCU_NO_INSTRUMENT
void *board_get_current_thread(void)
{
	return xTaskGetCurrentTaskHandle();
}

LIBMCU_NO_INSTRUMENT
unsigned long board_get_time_since_boot_ms(void)
{
	static uint64_t elapsed_ticks;
	static uint32_t previous_count;

	taskENTER_CRITICAL();

	const uint32_t count = xTaskGetTickCount();
	elapsed_ticks += count - previous_count;
	previous_count = count;

	taskEXIT_CRITICAL();

	return (unsigned long)((elapsed_ticks * 1000) / configTICK_RATE_HZ);
}

LIBMCU_NO_INSTRUMENT
unsigned long board_get_current_stack_watermark(void)
{
	return (unsigned long)uxTaskGetStackHighWaterMark(NULL) * sizeof(long);
}

LIBMCU_NO_INSTRUMENT
unsigned long board_get_heap_watermark(void)
{
	return (unsigned long)xPortGetMinimumEverFreeHeapSize();
}

LIBMCU_NO_INSTRUMENT
unsigned long board_get_free_heap_bytes(void)
{
	return (unsigned long)xPortGetFreeHeapSize();
}

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
