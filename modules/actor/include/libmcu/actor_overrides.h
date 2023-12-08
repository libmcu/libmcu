/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_ACTOR_OVERRIDES_H
#define LIBMCU_ACTOR_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

void actor_lock(void);
void actor_unlock(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_ACTOR_OVERRIDES_H */
