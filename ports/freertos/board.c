/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/board.h"
#include "libmcu/compiler.h"

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

	UBaseType_t intr_status;

	if (xPortIsInsideInterrupt()) {
		intr_status = taskENTER_CRITICAL_FROM_ISR();
	} else {
		taskENTER_CRITICAL();
	}

	const uint32_t count = xTaskGetTickCount();
	elapsed_ticks += count - previous_count;
	previous_count = count;

	if (xPortIsInsideInterrupt()) {
		taskEXIT_CRITICAL_FROM_ISR(intr_status);
	} else {
		taskEXIT_CRITICAL();
	}

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
