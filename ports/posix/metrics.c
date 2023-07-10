/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/metrics.h"
#include <pthread.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void metrics_lock(void)
{
	pthread_mutex_lock(&lock);
}

void metrics_unlock(void)
{
	pthread_mutex_unlock(&lock);
}
