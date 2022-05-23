#include "libmcu/posix/semaphore.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "libmcu/compiler.h"

#define SEM_MAX				0x7FFFU

struct semaphore {
	SemaphoreHandle_t handle;
};

int sem_init(sem_t *sem, int LIBMCU_UNUSED pshared, unsigned int value)
{
	struct semaphore *psem = (struct semaphore *)sem;

	psem->handle = xSemaphoreCreateCounting(SEM_MAX, value);
	if (psem->handle == NULL) {
		return -1;
	}

	return 0;
}

int sem_destroy(sem_t *sem)
{
	struct semaphore *psem = (struct semaphore *)sem;
	vSemaphoreDelete(psem->handle);
	return 0;
}

int sem_wait(sem_t *sem)
{
	struct semaphore *psem = (struct semaphore *)sem;
	return xSemaphoreTake(psem->handle, portMAX_DELAY) == pdPASS? 0 : -1;
}

int sem_timedwait(sem_t *sem, unsigned int timeout_ms)
{
	struct semaphore *psem = (struct semaphore *)sem;
	unsigned int ticks = pdMS_TO_TICKS(timeout_ms);
	return xSemaphoreTake(psem->handle, ticks) == pdPASS? 0 : -1;
}

int sem_trywait(sem_t *sem)
{
	struct semaphore *psem = (struct semaphore *)sem;
	return xSemaphoreTake(psem->handle, 0) == pdPASS? 0 : -1;
}

int sem_getvalue(sem_t *sem, int *sval)
{
	struct semaphore *psem = (struct semaphore *)sem;
	*sval = (int)uxSemaphoreGetCount(psem->handle);
	return 0;
}

int sem_post(sem_t *sem)
{
	struct semaphore *psem = (struct semaphore *)sem;
	return xSemaphoreGive(psem->handle) == pdPASS? 0 : -1;
}

int sem_post_nointr(sem_t *sem)
{
	struct semaphore *psem = (struct semaphore *)sem;
	return xSemaphoreGiveFromISR(psem->handle, NULL) == pdPASS? 0 : -1;
}
