#include "libmcu/button_overrides.h"
#include "libmcu/compiler.h"
#include <pthread.h>

static pthread_mutex_t lock_handle = PTHREAD_MUTEX_INITIALIZER;

LIBMCU_WEAK
void button_lock(void)
{
	pthread_mutex_lock(&lock_handle);
}

LIBMCU_WEAK
void button_unlock(void)
{
	pthread_mutex_unlock(&lock_handle);
}
