/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LOGGING_OVERRIDES_H
#define LOGGING_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

void logging_lock_init(void);
void logging_lock(void);
void logging_unlock(void);

#if defined(__cplusplus)
}
#endif

#endif /* LOGGING_OVERRIDES_H */
