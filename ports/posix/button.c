/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/button_overrides.h"
#include <pthread.h>

static pthread_mutex_t lock_handle = PTHREAD_MUTEX_INITIALIZER;

void button_lock(void)
{
	pthread_mutex_lock(&lock_handle);
}

void button_unlock(void)
{
	pthread_mutex_unlock(&lock_handle);
}
