/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/board.h"
#include "libmcu/compiler.h"
#include "libmcu/assert.h"

LIBMCU_WEAK
const char *board_get_version_string(void)
{
	return def2str(VERSION);
}

LIBMCU_WEAK
const char *board_get_build_date_string(void)
{
#if defined(BUILD_DATE)
	return BUILD_DATE;
#else
	return __DATE__;
#endif
}

LIBMCU_WEAK
const char *board_get_serial_number_string(void)
{
	return "";
}

LIBMCU_WEAK
const char *board_name(void)
{
	return "";
}

const char *board_get_reboot_reason_string(board_reboot_reason_t reason)
{
	static const char *reasons[] = {
		"Unknown",
		"Power On",
		"Power Pin",
		"Software",
		"Panic",
		"Watchdog",
		"Interrupt Watchdog",
		"Task Watchdog",
		"Deep Sleep",
		"Brownout",
		"SDIO",
	};

	return reasons[reason];
}

LIBMCU_WEAK
int board_reset_factory(void)
{
	return 0;
}

LIBMCU_WEAK
void board_reboot(void)
{
	assert(0);
}

LIBMCU_WEAK
void board_init(void)
{
}

LIBMCU_WEAK
LIBMCU_NO_INSTRUMENT
unsigned long board_get_time_since_boot_ms(void)
{
	return 0;
}

LIBMCU_WEAK
uint32_t board_random(void)
{
	return 0;
}

LIBMCU_WEAK
unsigned long board_get_free_heap_bytes(void)
{
	return 0;
}

LIBMCU_WEAK
unsigned long board_get_heap_watermark(void)
{
	return 0;
}

LIBMCU_WEAK
LIBMCU_NO_INSTRUMENT
unsigned long board_get_current_stack_watermark(void)
{
	return 0;
}

LIBMCU_WEAK
LIBMCU_NO_INSTRUMENT
void *board_get_current_thread(void)
{
	return 0;
}

LIBMCU_WEAK
uint8_t board_cpuload(int core_id, uint32_t period_sec)
{
	unused(core_id);
	unused(period_sec);
	return 0;
}

LIBMCU_WEAK
int board_get_revision(void)
{
	return 0;
}
