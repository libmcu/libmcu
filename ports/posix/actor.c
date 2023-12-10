/*
 * SPDX-FileCopyrightText: 2023 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/actor_overrides.h"
#include <pthread.h>

static pthread_mutex_t fallback_lock = PTHREAD_MUTEX_INITIALIZER;

void actor_lock(void)
{
	pthread_mutex_lock(&fallback_lock);
}

void actor_unlock(void)
{
	pthread_mutex_unlock(&fallback_lock);
}
