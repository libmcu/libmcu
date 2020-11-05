#include "CppUTestExt/MockSupport.h"
#include "semaphore.h"

struct semaphore {
	int count;
};

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
	struct semaphore *psem = (struct semaphore *)sem;
	psem->count = (int)value;

	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

int sem_destroy(sem_t *sem)
{
	struct semaphore *psem = (struct semaphore *)sem;
	psem->count = 0;

	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

int sem_wait(sem_t *sem)
{
	struct semaphore *psem = (struct semaphore *)sem;
	psem->count--;

	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

//TODO: implement sem_timedwait() and sem_trywait()
//int sem_timedwait(sem_t *sem, unsigned int timeout_ms)
//int sem_trywait(sem_t *sem)

int sem_post(sem_t *sem)
{
	struct semaphore *psem = (struct semaphore *)sem;
	psem->count++;

	return mock().actualCall(__func__)
		.returnIntValueOrDefault(0);
}

int sem_getvalue(sem_t *sem, int *sval)
{
	struct semaphore *psem = (struct semaphore *)sem;

	mock().actualCall(__func__);
	if (mock().hasReturnValue()) {
		return mock().returnIntValueOrDefault(0);
	}

	return psem->count;
}
