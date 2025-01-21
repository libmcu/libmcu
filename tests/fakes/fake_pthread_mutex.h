/*
 * SPDX-FileCopyrightText: 2020 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef FAKE_PTHREAD_MUTEX_H
#define FAKE_PTHREAD_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

bool fake_pthread_mutex_is_balanced(pthread_mutex_t *mutex);

#ifdef __cplusplus
}
#endif

#endif /* FAKE_PTHREAD_MUTEX_H */
