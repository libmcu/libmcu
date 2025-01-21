/*
 * SPDX-FileCopyrightText: 2022 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_AO_OVERRIDES_H
#define LIBMCU_AO_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

void ao_lock(void *ctx);
void ao_unlock(void *ctx);

void ao_timer_lock(void);
void ao_timer_unlock(void);
void ao_timer_lock_init(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_AO_OVERRIDES_H */
