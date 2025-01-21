/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BUTTON_OVERRIDES_H
#define LIBMCU_BUTTON_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

void button_lock(void);
void button_unlock(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BUTTON_OVERRIDES_H */
