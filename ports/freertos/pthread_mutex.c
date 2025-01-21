/*
 * SPDX-FileCopyrightText: 2021 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include <pthread.h>
#include <errno.h>
#include <time.h>

#include "libmcu/assert.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static int pthread_mutex_lock_internal(SemaphoreHandle_t sema, TickType_t timeout)
{
	if (xSemaphoreTake(sema, timeout) != pdTRUE) {
		return -EBUSY;
	}

	return 0;
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	(void)attr;

	SemaphoreHandle_t sema;

	if (mutex == NULL) {
		return -EINVAL;
	}
	if ((sema = xSemaphoreCreateMutex()) == NULL) {
		return -EINVAL;
	}

	*mutex = (pthread_mutex_t)sema;

	return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	if (mutex == NULL) {
		return -EINVAL;
	}

	SemaphoreHandle_t sema = (SemaphoreHandle_t)*mutex;

	vSemaphoreDelete(sema);

	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	if (mutex == NULL) {
		return -EINVAL;
	}

	SemaphoreHandle_t sema = (SemaphoreHandle_t)*mutex;

	return pthread_mutex_lock_internal(sema, portMAX_DELAY);
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	if (mutex == NULL) {
		return -EINVAL;
	}

	SemaphoreHandle_t sema = (SemaphoreHandle_t)*mutex;

	return pthread_mutex_lock_internal(sema, 0);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	if (mutex == NULL) {
		return -EINVAL;
	}

	SemaphoreHandle_t sema = (SemaphoreHandle_t)*mutex;

        int ok = xSemaphoreGive(sema);
	assert(ok == pdTRUE);

	return 0;
}

#if defined(_POSIX_TIMEOUTS)
int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *timeout)
{
	if (mutex == NULL || timeout == NULL) {
		return -EINVAL;
	}

	SemaphoreHandle_t sema = (SemaphoreHandle_t)*mutex;

	struct timespec currtime;
	clock_gettime(CLOCK_REALTIME, &currtime);
	TickType_t timeout_tick = (TickType_t)(
			(timeout->tv_sec - currtime.tv_sec) * 1000 +
			(timeout->tv_nsec - currtime.tv_nsec) / 1000000);

	if (pthread_mutex_lock_internal(sema, timeout_tick / configTICK_RATE_HZ) == -EBUSY) {
		return -ETIMEDOUT;
	}

	return 0;
}
#endif
