#include "libmcu/ao_overrides.h"
#include "libmcu/compiler.h"
#include <pthread.h>

LIBMCU_WEAK
void ao_lock(void *lock_handle)
{
	pthread_mutex_lock(lock_handle);
}

LIBMCU_WEAK
void ao_unlock(void *lock_handle)
{
	pthread_mutex_unlock(lock_handle);
}

LIBMCU_WEAK
int ao_lock_init(void *lock_handle, void *arg)
{
	return pthread_mutex_init(lock_handle, arg);
}
