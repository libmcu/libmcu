#include "libmcu/ao_overrides.h"
#include "libmcu/compiler.h"
#include <pthread.h>

static pthread_mutex_t fallback_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t timer_lock;

LIBMCU_WEAK
void ao_lock(void *ctx)
{
	unused(ctx);
	pthread_mutex_lock(&fallback_lock);
}

LIBMCU_WEAK
void ao_unlock(void *ctx)
{
	unused(ctx);
	pthread_mutex_unlock(&fallback_lock);
}

LIBMCU_WEAK
void ao_timer_lock(void)
{
	pthread_mutex_lock(&timer_lock);
}

LIBMCU_WEAK
void ao_timer_unlock(void)
{
	pthread_mutex_unlock(&timer_lock);
}

LIBMCU_WEAK
void ao_timer_lock_init(void)
{
	pthread_mutex_init(&timer_lock, NULL);
}
