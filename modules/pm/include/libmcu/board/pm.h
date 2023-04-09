/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BOARD_PM_H
#define LIBMCU_BOARD_PM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/pm.h"

int pm_board_enter(pm_mode_t mode);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BOARD_PM_H */
