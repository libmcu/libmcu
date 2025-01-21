/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_JOBPOOL_H
#define LIBMCU_JOBPOOL_H

#include <stdbool.h>

bool jobpool_init(void);
bool jobpool_schedule(void (*job)(void *context), void *job_context);
unsigned int jobpool_count(void);

#endif /* LIBMCU_JOBPOOL_H */
