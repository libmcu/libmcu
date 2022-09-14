/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/logging.h"
#include <pthread.h>

static pthread_mutex_t lock;

void logging_lock_init(void)
{
	pthread_mutex_init(&lock, NULL);
}

void logging_lock(void)
{
	pthread_mutex_lock(&lock);
}

void logging_unlock(void)
{
	pthread_mutex_unlock(&lock);
}
