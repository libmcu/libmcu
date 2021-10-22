#include <pthread.h>
#include <errno.h>

#include "libmcu/assert.h"
#include "FreeRTOS.h"
#include "task.h"

#if !defined(PTHREAD_STACK_MIN)
#define PTHREAD_STACK_MIN			128
#endif
#if !defined(DEFAULT_PTHREAD_STACK_SIZE)
#define DEFAULT_PTHREAD_STACK_SIZE		256
#endif
#if !defined(DEFAULT_PTHREAD_PRIORITY)
#define DEFAULT_PTHREAD_PRIORITY		5
#endif
#if !defined(DEFAULT_PTHREAD_NAME)
#define DEFAULT_PTHREAD_NAME			"pthread"
#endif

struct task_ctx {
	void *(*task)(void *);
	void *arg;
};

static void task_wrapper(void *param)
{
	const struct task_ctx *ctx = (const struct task_ctx *)param;
	void *(*task)(void *) = ctx->task;
	void *arg = ctx->arg;

	vPortFree(param);

	void *res = task(arg);
	(void)res;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		void *(*start_routine)(void *), void *arg)
{
	size_t stack_size = DEFAULT_PTHREAD_STACK_SIZE;
	const char *name = DEFAULT_PTHREAD_NAME;
	int pri = DEFAULT_PTHREAD_PRIORITY;
	struct task_ctx *ctx;

	if (thread == NULL || start_routine == NULL) {
		return -EINVAL;
	}
	if ((ctx = pvPortMalloc(sizeof(*ctx))) == NULL) {
		return -ENOMEM;
	}

	ctx->task = start_routine;
	ctx->arg = arg;

	if (attr != NULL) {
		stack_size = (size_t)attr->stacksize;
		pri = attr->schedparam.sched_priority;
	}

	TaskHandle_t task_handle = NULL;
	BaseType_t ok = xTaskCreate(task_wrapper,
			name,
			(uint16_t)stack_size,
			ctx,
			(uint16_t)pri,
			&task_handle);

	if (ok != pdPASS) {
		return -EAGAIN;
	}

	// TODO: make a list of tasks

	*thread = (pthread_t)task_handle;

	return 0;
}

void pthread_exit(void *retval)
{
	(void)retval;

	vTaskDelete(NULL);

	assert(0); /* should never called */

	while (1) { /* hang */ }
}

pthread_t pthread_self(void)
{
	TaskHandle_t handle = xTaskGetCurrentTaskHandle();
	return (pthread_t)handle;
}

#if 0
int pthread_join(pthread_t thread, void **status)
int pthread_detach(pthread_t thread);
int pthread_cancel(pthread_t thread);
#endif

int pthread_attr_init(pthread_attr_t *attr)
{
	if (attr == NULL) {
		return -EINVAL;
	}

	attr->stacksize = DEFAULT_PTHREAD_STACK_SIZE;
	attr->detachstate = PTHREAD_CREATE_JOINABLE;
	attr->schedparam.sched_priority = DEFAULT_PTHREAD_PRIORITY;

	return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
	(void)attr;
	return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
	if (attr == NULL) {
		return -EINVAL;
	}

	switch (detachstate) {
	case PTHREAD_CREATE_DETACHED:
		attr->detachstate = PTHREAD_CREATE_DETACHED;
		break;
	case PTHREAD_CREATE_JOINABLE:
		attr->detachstate = PTHREAD_CREATE_JOINABLE;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
	if (attr == NULL || stacksize < PTHREAD_STACK_MIN) {
		return -EINVAL;
	}

	attr->stacksize = (int)stacksize;

	return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
	if (attr == NULL || stacksize == NULL) {
		return -EINVAL;
	}

	*stacksize = (size_t)attr->stacksize;

	return 0;
}

#if 0
int pthread_attr_setschedparam(pthread_attr_t *attr,
		const struct sched_param *param)
int pthread_attr_getschedparam(const pthread_attr_t *attr,
		struct sched_param *param)
#endif

#if 0
// not posix
int pthread_attr_getname_np(const pthread_attr_t *attr, char *name)
int pthread_attr_setname_np(pthread_attr_t *attr, const char *name)
#endif
