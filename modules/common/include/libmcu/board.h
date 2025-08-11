/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
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
	BOARD_REBOOT_DEBUGGER,
} board_reboot_reason_t;

typedef void (*board_idle_hook_t)(void);

void board_init(void);
void board_reboot(void);
int board_reset_factory(void);

const char *board_get_version_string(void);
const char *board_get_build_date_string(void);
const char *board_get_serial_number_string(void);
const char *board_name(void);

board_reboot_reason_t board_get_reboot_reason(void);
const char *board_get_reboot_reason_string(board_reboot_reason_t reason);

uint32_t board_get_time_since_boot_ms(void);
uint64_t board_get_time_since_boot_us(void);

int board_register_idle_hook(int opt, board_idle_hook_t func);
uint32_t board_random(void);

/**
 * @brief Get the CPU load for a specific core over a certain period.
 *
 * This function returns the CPU load percentage for a specific core identified
 * by `core_id` over a period of time specified by `period_sec` in seconds.
 *
 * @param[in] core_id The ID of the core for which to get the CPU load.
 * @param[n] period_sec The period over which to measure the CPU load in seconds.
 * @return The CPU load percentage for the specified core over the specified
 *         period.
 */
uint8_t board_cpuload(int core_id, uint32_t period_sec);

uint32_t board_get_free_heap_bytes(void);
uint32_t board_get_total_heap_bytes(void);
uint32_t board_get_heap_watermark(void);
uint32_t board_get_current_stack_watermark(void);

void *board_get_current_thread(void);

/**
 * @brief Get the revision number of the board.
 *
 * This function returns the revision number of the board. The revision number
 * is a unique identifier that represents the version of the board.
 *
 * @return The revision number of the board.
 */
int board_get_revision(void);

/**
 * @brief Switches the board to factory mode.
 *
 * This function transitions the board into factory mode, which is typically
 * used for manufacturing or provisioning purposes.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int board_switch_to_factory(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BOARD_H */
