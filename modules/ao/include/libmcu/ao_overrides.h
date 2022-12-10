/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_AO_OVERRIDES_H
#define LIBMCU_AO_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

void ao_lock(void *lock_handle);
void ao_unlock(void *lock_handle);
int ao_lock_init(void *lock_handle, void *arg);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_AO_OVERRIDES_H */
