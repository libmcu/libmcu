/*
 * SPDX-FileCopyrightText: 2025 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include <dispatch/dispatch.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_SEMAPHORES		64

typedef struct {
	int used;
	sem_t *key;
	dispatch_semaphore_t value;
} mac_sem_entry_t;

static mac_sem_entry_t table[MAX_SEMAPHORES];
static pthread_mutex_t table_lock = PTHREAD_MUTEX_INITIALIZER;

static int insert(sem_t *key, dispatch_semaphore_t sem)
{
	pthread_mutex_lock(&table_lock);
	for (int i = 0; i < MAX_SEMAPHORES; ++i) {
		if (!table[i].used) {
			table[i].used = 1;
			table[i].key = key;
			table[i].value = sem;
			pthread_mutex_unlock(&table_lock);
			return 0;
		}
	}
	pthread_mutex_unlock(&table_lock);
	return -1;
}

static dispatch_semaphore_t lookup(sem_t *key)
{
	dispatch_semaphore_t result = NULL;
	pthread_mutex_lock(&table_lock);
	for (int i = 0; i < MAX_SEMAPHORES; ++i) {
		if (table[i].used && table[i].key == key) {
			result = table[i].value;
			break;
		}
	}
	pthread_mutex_unlock(&table_lock);
	return result;
}

static void remove_entry(sem_t *key)
{
	pthread_mutex_lock(&table_lock);
	for (int i = 0; i < MAX_SEMAPHORES; ++i) {
		if (table[i].used && table[i].key == key) {
			table[i].used = 0;
			table[i].key = NULL;
			table[i].value = NULL;
			break;
		}
	}
	pthread_mutex_unlock(&table_lock);
}

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
	if (pshared != 0) {
		errno = ENOTSUP;
		return -1;
	}

	dispatch_semaphore_t dsem = dispatch_semaphore_create(value);
	if (!dsem) {
		errno = ENOMEM;
		return -1;
	}

	if (insert(sem, dsem) != 0) {
		dispatch_release(dsem);
		errno = ENOMEM;
		return -1;
	}

	return 0;
}

int sem_destroy(sem_t *sem)
{
	remove_entry(sem);
	return 0;
}

int sem_post(sem_t *sem)
{
	dispatch_semaphore_t dsem = lookup(sem);
	if (!dsem) {
		errno = EINVAL;
		return -1;
	}
	dispatch_semaphore_signal(dsem);
	return 0;
}

int sem_wait(sem_t *sem)
{
	dispatch_semaphore_t dsem = lookup(sem);
	if (!dsem) {
		errno = EINVAL;
		return -1;
	}
	dispatch_semaphore_wait(dsem, DISPATCH_TIME_FOREVER);
	return 0;
}
