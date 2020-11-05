#include <pthread.h>
#include "fakes/fake_pthread_mutex.h"

struct fake_mutex {
	int count;
};

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
	struct fake_mutex *p = (struct fake_mutex *)mutex;
	p->count = 1;
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	struct fake_mutex *p = (struct fake_mutex *)mutex;
	p->count--;
	return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	struct fake_mutex *p = (struct fake_mutex *)mutex;
	p->count++;
	return 0;
}

bool fake_pthread_mutex_is_balanced(pthread_mutex_t *mutex)
{
	struct fake_mutex *p = (struct fake_mutex *)mutex;
	return p->count == 1;
}
