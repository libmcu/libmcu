/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_RETRY_OVERRIDES_H
#define LIBMCU_RETRY_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

int retry_generate_random(void);
void retry_sleep_ms(unsigned int msec);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_RETRY_OVERRIDES_H */
