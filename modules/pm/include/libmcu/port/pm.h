/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_PORT_PM_H
#define LIBMCU_PORT_PM_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "libmcu/pm.h"

int pm_port_enter(pm_mode_t mode, uint32_t duration_ms);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_PORT_PM_H */
