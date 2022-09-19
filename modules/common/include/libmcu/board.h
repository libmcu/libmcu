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

void board_init(void);
void board_reboot(void);
int board_reset_factory(void);

const char *board_get_version_string(void);
const char *board_get_build_date_string(void);

const char *board_get_serial_number_string(void);
const char *board_get_reboot_reason_string(void);

unsigned int board_get_free_heap_bytes(void);
unsigned int board_get_total_heap_bytes(void);
unsigned int board_get_heap_watermark(void);
unsigned int board_get_current_stack_watermark(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BOARD_H */
