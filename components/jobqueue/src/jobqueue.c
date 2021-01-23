#include "libmcu/jobqueue.h"

#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#include "libmcu/list.h"
#include "libmcu/logging.h"

struct jobqueue {
	pthread_mutex_t lock;
	struct list job_list;
	sem_t job_queue;
	jobqueue_attr_t attr;
	uint8_t active_threads;
	uint8_t max_concurrent_jobs;
	uint8_t nr_running;
};

struct job {
	struct list entry;
	struct list *pool;
	job_callback_t callback;
	void *context;
};

static inline bool is_job_scheduled(const jobqueue_t *pool, const struct job *job)
{
	struct list *p;
	list_for_each(p, &pool->job_list) {
		if (p == &job->entry) {
			return true;
		}
	}
	return false;
}

static inline uint8_t count_scheduled(const jobqueue_t *pool)
{
	uint8_t count = 0;
	struct list *p;

	list_for_each(p, &pool->job_list) {
		count++;
	}

	return count;
}

static inline uint8_t count_running(const jobqueue_t *pool)
{
	return pool->nr_running;
}

static inline uint8_t job_count_internal(const jobqueue_t *pool)
{
	return (uint8_t)(count_scheduled(pool) + count_running(pool));
}

static inline void job_delete_internal(jobqueue_t *pool, const struct job *job)
{
	struct list *p;
	list_for_each(p, &pool->job_list) {
		if (p == &job->entry) {
			list_del(p, &pool->job_list);
			break;
		}
	}
}

static struct job *get_job_scheduled_detaching(jobqueue_t *pool)
{
	if (count_scheduled(pool) == 0) {
		return NULL;
	}

	struct job *job = list_entry(pool->job_list.next, struct job, entry);
	list_del(&job->entry, &pool->job_list);

	return job;
}

static bool jobqueue_process(jobqueue_t *pool)
{
	bool busy = true;
	struct job *job;

	sem_wait(&pool->job_queue);

	pthread_mutex_lock(&pool->lock);
	{
		job = get_job_scheduled_detaching(pool);
		pool->nr_running++;
	}
	pthread_mutex_unlock(&pool->lock);

	if (job && job->callback) {
		job->callback(job->context);
	}

	pthread_mutex_lock(&pool->lock);
	{
		pool->nr_running--;

		if (job_count_internal(pool) <= pool->active_threads &&
				pool->active_threads > pool->attr.min_threads) {
			pool->active_threads--;
			busy = false;
		}
	}
	pthread_mutex_unlock(&pool->lock);

	return busy;
}

static void *jobqueue_task(void *e)
{
	info("new thread created");

	while (jobqueue_process(e)) {
	}

	info("terminating thread");

#if !defined(UNITTEST)
	pthread_exit(NULL);
#endif
	return NULL;
}

static inline job_error_t job_schedule_internal(jobqueue_t *pool, struct job *job)
{
	uint8_t nr_jobs = job_count_internal(pool);
	if (nr_jobs >= pool->max_concurrent_jobs) {
		return JOB_FULL;
	}

	if (!is_job_scheduled(pool, job)) {
		list_add_tail(&job->entry, &pool->job_list);
		sem_post(&pool->job_queue);
	}

	if (nr_jobs >= pool->active_threads
			&& pool->active_threads < pool->attr.max_threads) {
		pthread_t thread;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, pool->attr.stack_size_bytes);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if (pthread_create(&thread, &attr, jobqueue_task, pool) != 0) {
			error("cannot create new thread");
			return JOB_ERROR;
		}
		pool->active_threads++;
	}

	return JOB_SUCCESS;
}

jobqueue_t *jobqueue_create(uint8_t max_concurrent_jobs)
{
	jobqueue_t *pool;

	if (!(pool = calloc(1, sizeof(*pool)))) {
		goto out_err;
	}
	if (sem_init(&pool->job_queue, 0, 0) != 0) {
		goto out_free_pool;
	}

	list_init(&pool->job_list);
	pthread_mutex_init(&pool->lock, NULL);
	pool->nr_running = 0;
	pool->active_threads = 0;
	pool->max_concurrent_jobs = max_concurrent_jobs;
	pool->attr = (jobqueue_attr_t) {
		.stack_size_bytes = JOBQUEUE_DEFAULT_STACK_SIZE,
		.min_threads = JOBQUEUE_DEFAULT_MIN_THREADS,
		.max_threads = JOBQUEUE_DEFAULT_MAX_THREADS,
		.priority = JOBQUEUE_DEFAULT_PRIORITY,
	};

	return pool;
out_free_pool:
	free(pool);
out_err:
	return NULL;
}

job_error_t jobqueue_set_attr(jobqueue_t *pool, const jobqueue_attr_t *attr)
{
	if (!pool || !attr) {
		return JOB_INVALID_PARAM;
	}

	pthread_mutex_lock(&pool->lock);
	{
		pool->attr = *attr;
	}
	pthread_mutex_unlock(&pool->lock);

	return JOB_SUCCESS;
}

job_error_t jobqueue_destroy(jobqueue_t *pool)
{
	if (!pool) {
		return JOB_INVALID_PARAM;
	}

	uint8_t jobs_left = job_count(pool);
	if (jobs_left != 0) {
		warn("%u jobs to be run exist", jobs_left);
	}

	pthread_mutex_lock(&pool->lock);
	{
		sem_destroy(&pool->job_queue);
	}
	pthread_mutex_unlock(&pool->lock);

	free(pool);

	return JOB_SUCCESS;
}

job_error_t job_init(jobqueue_t *pool, job_t *job,
		job_callback_t callback, void *context)
{
	if (!pool || !job) {
		return JOB_INVALID_PARAM;
	}

	struct job *p = (struct job *)job;

	p->pool = &pool->job_list;
	p->callback = callback;
	p->context = context;

	return JOB_SUCCESS;
}

job_error_t job_delete(jobqueue_t *pool, job_t *job)
{
	if (!pool || !job) {
		return JOB_INVALID_PARAM;
	}

	pthread_mutex_lock(&pool->lock);
	{
		job_delete_internal(pool, (struct job *)job);
	}
	pthread_mutex_unlock(&pool->lock);

	return JOB_SUCCESS;
}

job_error_t job_schedule(jobqueue_t *pool, job_t *job)
{
	if (!pool || !job) {
		return JOB_INVALID_PARAM;
	}

	job_error_t err;

	pthread_mutex_lock(&pool->lock);
	{
		err = job_schedule_internal(pool, (struct job *)job);
	}
	pthread_mutex_unlock(&pool->lock);

	return err;
}

uint8_t job_count(jobqueue_t *pool)
{
	uint8_t count;

	pthread_mutex_lock(&pool->lock);
	{
		count = job_count_internal(pool);
	}
	pthread_mutex_unlock(&pool->lock);

	int semcnt = 0;
	sem_getvalue(&pool->job_queue, &semcnt);
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
	case JOB_INVALID_PARAM:
		return "invalid parameters";
	case JOB_FULL:
		return "no room for a new job";
	case JOB_ERROR:
	default:
		break;
	}

	return "unknown error";
}
