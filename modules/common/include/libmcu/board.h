/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BOARD_H
#define LIBMCU_BOARD_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

typedef enum {
	BOARD_REBOOT_UNKNOWN,
	BOARD_REBOOT_POWER,
	BOARD_REBOOT_PIN,
	BOARD_REBOOT_SOFT,
	BOARD_REBOOT_PANIC,
	BOARD_REBOOT_WDT,
	BOARD_REBOOT_WDT_INT,
	BOARD_REBOOT_WDT_TASK,
	BOARD_REBOOT_DEEPSLEEP,
	BOARD_REBOOT_BROWNOUT,
	BOARD_REBOOT_SDIO,
} board_reboot_reason_t;

void board_init(void);
void board_reboot(void);
int board_reset_factory(void);

const char *board_get_version_string(void);
const char *board_get_build_date_string(void);
const char *board_get_serial_number_string(void);

board_reboot_reason_t board_get_reboot_reason(void);
const char *board_get_reboot_reason_string(board_reboot_reason_t reason);

unsigned long board_get_time_since_boot_ms(void);

long board_random(void);

/**
 * @brief Get overall CPU usage
 *
 * @param core_id the core's identifier
 * @return percentage of 0 to 100
 */
uint8_t board_cpuload(int core_id);

unsigned long board_get_free_heap_bytes(void);
unsigned long board_get_total_heap_bytes(void);
unsigned long board_get_heap_watermark(void);
unsigned long board_get_current_stack_watermark(void);

void *board_get_current_thread(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BOARD_H */
