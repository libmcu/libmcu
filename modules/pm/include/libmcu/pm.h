/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_PM_H
#define LIBMCU_PM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

#if !defined(PM_CALLBACK_MAXLEN)
#define PM_CALLBACK_MAXLEN		8U
#endif

#define pm_init				libmcu_pm_init

typedef enum pm_mode {
	PM_SLEEP,
	PM_SLEEP_DEEP,
	PM_SLEEP_SHIP,
	PM_RESET_SOFT,
	PM_RESET_HARD,
	PM_SHUTDOWN,
} pm_mode_t;

typedef void (*pm_callback_t)(void *ctx);

int pm_enter(pm_mode_t mode, uint32_t duration_ms);
int pm_register_entry_callback(pm_mode_t mode, int8_t priority,
		pm_callback_t func, void *arg);
int pm_register_exit_callback(pm_mode_t mode, int8_t priority,
		pm_callback_t func, void *arg);
int pm_unregister_entry_callback(pm_mode_t mode, int8_t priority,
		pm_callback_t func);
int pm_unregister_exit_callback(pm_mode_t mode, int8_t priority,
		pm_callback_t func);
void pm_init(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_PM_H */
