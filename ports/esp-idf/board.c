/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/board.h"
#include "libmcu/compiler.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_timer.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_random.h"

LIBMCU_NO_INSTRUMENT
void *board_get_current_thread(void)
{
	return xTaskGetCurrentTaskHandle();
}

LIBMCU_NO_INSTRUMENT
unsigned long board_get_time_since_boot_ms(void)
{
	const int64_t elapsed_usec = esp_timer_get_time();
	return (unsigned long)(elapsed_usec / 1000);
}

LIBMCU_NO_INSTRUMENT
unsigned long board_get_current_stack_watermark(void)
{
	return (unsigned long)uxTaskGetStackHighWaterMark(NULL);
}

LIBMCU_NO_INSTRUMENT
unsigned long board_get_heap_watermark(void)
{
	return esp_get_minimum_free_heap_size();
}

LIBMCU_NO_INSTRUMENT
unsigned long board_get_free_heap_bytes(void)
{
	return esp_get_free_heap_size();
}

board_reboot_reason_t board_get_reboot_reason(void)
{
	switch (esp_reset_reason()) {
	case ESP_RST_POWERON:
		return BOARD_REBOOT_POWER;
	case ESP_RST_EXT:
		return BOARD_REBOOT_PIN;
	case ESP_RST_SW:
		return BOARD_REBOOT_SOFT;
	case ESP_RST_PANIC:
		return BOARD_REBOOT_PANIC;
	case ESP_RST_INT_WDT:
		return BOARD_REBOOT_WDT_INT;
	case ESP_RST_TASK_WDT:
		return BOARD_REBOOT_WDT_TASK;
	case ESP_RST_WDT:
		return BOARD_REBOOT_WDT;
	case ESP_RST_DEEPSLEEP:
		return BOARD_REBOOT_DEEPSLEEP;
	case ESP_RST_BROWNOUT:
		return BOARD_REBOOT_BROWNOUT;
	case ESP_RST_SDIO:
		return BOARD_REBOOT_SDIO;
	default:
		return BOARD_REBOOT_UNKNOWN;
	}
}

uint32_t board_random(void)
{
	return esp_random();
}

void board_reboot(void)
{
	esp_restart();
}
