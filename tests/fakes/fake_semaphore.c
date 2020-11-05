#include <semaphore.h>
#include <stdint.h>

struct semaphore {
	int count;
};

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
	struct semaphore *psem = (struct semaphore *)sem;
	psem->count = (int)value;
	return 0;
}

int sem_destroy(sem_t *sem)
{
	struct semaphore *psem = (struct semaphore *)sem;
	psem->count = 0;
	return 0;
}

int sem_wait(sem_t *sem)
{
	struct semaphore *psem = (struct semaphore *)sem;
	psem->count--;
	return 0;
}

//int sem_timedwait(sem_t *sem, unsigned int timeout_ms)
//int sem_trywait(sem_t *sem)

int sem_post(sem_t *sem)
{
	struct semaphore *psem = (struct semaphore *)sem;
	psem->count++;
	return 0;
}

int sem_getvalue(sem_t *sem, int *sval)
{
	struct semaphore *psem = (struct semaphore *)sem;
	return psem->count;
}
