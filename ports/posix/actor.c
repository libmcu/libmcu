/*
 * SPDX-FileCopyrightText: 2023 권경환 Kyunghwan Kwon <k@libmcu.org>
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
