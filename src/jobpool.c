#include "jobpool.h"

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#include "list.h"
#include "logger.h"
#include "compiler.h"

#define DEFAULT_STACK_SIZE			3072
#define DEFAULT_MIN_THREADS			1
#define DEFAULT_MAX_THREADS			1
#define DEFAULT_PRIORITY			5

typedef enum {
	JOB_STATE_INITIALIZED = 0,
	JOB_STATE_SCHEDULED,
	JOB_STATE_COMPLETED,
	JOB_STATE_DELETED,
	JOB_STATE_UNKNOWN,
} job_state_t;

struct jobpool {
	pthread_mutex_t lock;
	struct list job_list;
	sem_t sem;
	jobpool_attr_t attr;
	uint8_t active_threads;
};

struct job {
	struct list entry;
	struct list *pool;
	job_state_t state;
	job_callback_t callback;
	job_context_t *context;
};

static inline uint8_t job_count_internal(jobpool_t *pool)
{
	uint8_t count = 0;
	struct list *p;

	list_for_each(p, &pool->job_list) {
		count++;
	}

	return count;
}

static bool jobpool_process(jobpool_t *pool)
{
	bool busy = true;
	struct job *job = NULL;

	sem_wait(&pool->sem);

	pthread_mutex_lock(&pool->lock);
	{
		uint8_t njobs_waiting = job_count_internal(pool);

		if (njobs_waiting) {
			job = list_entry(pool->job_list.next, struct job, entry);
			list_del(&job->entry, &pool->job_list);
			job->state = JOB_STATE_COMPLETED;
		}

		if (njobs_waiting <= pool->active_threads
				&& pool->active_threads > pool->attr.min_threads) {
			pool->active_threads--;
			busy = false;
		}
	}
	pthread_mutex_unlock(&pool->lock);

	if (job && job->callback) {
		job->callback(job->context);
		// reschedule if JOB_RETRY returned?
	}

	return busy;
}

static void *jobpool_task(void *e)
{
	while (jobpool_process(e));

	pthread_exit(NULL);
}

jobpool_t *jobpool_create(unsigned int max_concurrent_jobs)
{
	jobpool_t *pool;

	if (!(pool = calloc(1, sizeof(*pool)))) {
		goto out;
	}
	if (sem_init(&pool->sem, 0, max_concurrent_jobs) != 0) {
		goto out_free_pool;
	}

	list_init(&pool->job_list);
	pthread_mutex_init(&pool->lock, NULL);
	pool->active_threads = 0;
	pool->attr = (jobpool_attr_t) {
		.stack_size_bytes = DEFAULT_STACK_SIZE,
		.min_threads = DEFAULT_MIN_THREADS,
		.max_threads = DEFAULT_MAX_THREADS,
		.priority = DEFAULT_PRIORITY,
	};

	return pool;
out_free_pool:
	free(pool);
out:
	return NULL;
}

job_error_t jobpool_set_attr(jobpool_t *pool, const jobpool_attr_t *attr)
{
	if (!pool || !attr) {
		return JOB_ERROR_PARAM;
	}

	pool->attr = *attr;

	return JOB_SUCCESS;
}

job_error_t jobpool_destroy(jobpool_t *pool)
{
	if (!pool) {
		return JOB_ERROR_PARAM;
	}

	if (job_count(pool) != 0) {
		warn("jobs to be run exist");
	}

	pthread_mutex_lock(&pool->lock);
	{
		sem_destroy(&pool->sem);
	}
	pthread_mutex_unlock(&pool->lock);

	free(pool);

	return JOB_SUCCESS;
}

job_error_t job_init(jobpool_t *pool, job_t *job, job_callback_t callback, job_context_t *context)
{
	if (!pool || !job) {
		return JOB_ERROR_PARAM;
	}

	struct job *pjob = (struct job *)job;

	pjob->pool = &pool->job_list;
	pjob->callback = callback;
	pjob->context = context;
	pjob->state = JOB_STATE_INITIALIZED;

	return JOB_SUCCESS;
}

job_error_t job_delete(jobpool_t *pool, job_t *job)
{
	if (!pool || !job) {
		return JOB_ERROR_PARAM;
	}

	pthread_mutex_lock(&pool->lock);
	{
		struct job *pjob = (struct job *)job;
		if (pjob->state == JOB_STATE_SCHEDULED) {
			struct list *p;
			list_for_each(p, &pool->job_list) {
				if (p == &pjob->entry) {
					list_del(p, &pool->job_list);
					pjob->state = JOB_STATE_DELETED;
					break;
				}
			}
		}
	}
	pthread_mutex_unlock(&pool->lock);

	return JOB_SUCCESS;
}

job_error_t job_schedule(jobpool_t *pool, job_t *job)
{
	if (!pool || !job) {
		return JOB_ERROR_PARAM;
	}

	pthread_mutex_lock(&pool->lock);
	{
		if (job_count_internal(pool) >= pool->active_threads
				&& pool->active_threads < pool->attr.max_threads) {
			pthread_t thread;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setstacksize(&attr, pool->attr.stack_size_bytes);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			if (pthread_create(&thread, &attr, jobpool_task, pool) == 0) {
				pool->active_threads++;
			}
		}

		struct job *pjob = (struct job *)job;
		if (pjob->state != JOB_STATE_SCHEDULED) {
			list_add_tail(&pjob->entry, &pool->job_list);
			pjob->state = JOB_STATE_SCHEDULED;
			sem_post(&pool->sem);
		}
	}
	pthread_mutex_unlock(&pool->lock);

	return JOB_SUCCESS;
}

uint8_t job_count(jobpool_t *pool)
{
	uint8_t count;

	pthread_mutex_lock(&pool->lock);
	{
		count = job_count_internal(pool);
	}
	pthread_mutex_unlock(&pool->lock);

	int semcnt = 0;
	sem_getvalue(&pool->sem, &semcnt);
	if (count != (uint8_t)semcnt) {
		error("count doesn't match %d - %d", count, semcnt);
	}

	return count;
}

const char *job_stringify_error(job_error_t error_code)
{
	switch (error_code) {
	case JOB_SUCCESS:
		return "success";
	case JOB_RETRY:
		return "retry";
	case JOB_ERROR_PARAM:
		return "wrong parameters";
	case JOB_ERROR:
		break;
	}

	return "unknown error";
}
