/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "libmcu/posix/semaphore.h"
#include "libmcu/compiler.h"

LIBMCU_WEAK
int sem_init(sem_t *sem, int pshared, unsigned int value)
{
	unused(sem);
	unused(pshared);
	unused(value);
	return 0;
}

LIBMCU_WEAK
int sem_destroy(sem_t *sem)
{
	unused(sem);
	return 0;
}

LIBMCU_WEAK
int sem_wait(sem_t *sem)
{
	unused(sem);
	return 0;
}

LIBMCU_WEAK
int sem_timedwait(sem_t *sem, unsigned int timeout_ms)
{
	unused(sem);
	unused(timeout_ms);
	return 0;
}

LIBMCU_WEAK
int sem_trywait(sem_t *sem)
{
	unused(sem);
	return 0;
}

LIBMCU_WEAK
int sem_getvalue(sem_t *sem, int *sval)
{
	unused(sem);
	unused(sval);
	return 0;
}

LIBMCU_WEAK
int sem_post(sem_t *sem)
{
	unused(sem);
	return 0;
}
