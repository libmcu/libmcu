/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#if defined(__APPLE__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wcast-align"

#include <semaphore.h>
#include <dispatch/dispatch.h>
#include <stdint.h>

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
	dispatch_semaphore_t *p = (dispatch_semaphore_t *)sem;
	*p = dispatch_semaphore_create(value);
	return 0;
}

int sem_wait(sem_t *sem)
{
	dispatch_semaphore_t *p = (dispatch_semaphore_t *)sem;
	dispatch_semaphore_wait(*p, DISPATCH_TIME_FOREVER);
	return 0;
}

int sem_post(sem_t *sem)
{
	dispatch_semaphore_t *p = (dispatch_semaphore_t *)sem;
	dispatch_semaphore_signal(*p);
	return 0;
}

#pragma clang diagnostic pop
#endif
