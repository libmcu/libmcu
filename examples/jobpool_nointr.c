#include "jobpool.h"
#include <stdatomic.h>
#include <semaphore.h>
#include "libmcu/jobqueue.h"

#if !defined(JOBPOOL_MAX_JOBS)
#define JOBPOOL_MAX_JOBS		8
#endif
#if !defined(JOBPOOL_MAX_THREADS)
#define JOBPOOL_MAX_THREADS		3
#endif

enum job_state {
	INACTIVE			= 0,
	PENDING,
	/* ACTIVE must greater than the number of threads that can call
	 * `jobpool_schedule()` concurrently for syncronization. */
	ACTIVE				= JOBPOOL_MAX_JOBS,
};

struct job_desc {
	job_static_t obj;
	int state;
	void (*run)(void *context);
	void *context;
};

static struct {
	jobqueue_t jobqueue;
	struct job_desc jobs[JOBPOOL_MAX_JOBS];
	unsigned int job_index;

	sem_t sema;
	job_static_t manager;
} m;

static void job_wrapper(void *context)
{
	struct job_desc *job = (struct job_desc *)context;

	job->run(job->context);

	job->state = INACTIVE;
}

static bool schedule_job(struct job_desc *job)
{
	if (job_create_static(m.jobqueue, &job->obj, job_wrapper, job)
			!= JOB_SUCCESS) {
		return false;
	}
	if (job_schedule(m.jobqueue, &job->obj) != JOB_SUCCESS) {
		return false;
	}

	job->state = ACTIVE;
	return true;
}

static bool is_pending(int state)
{
	if (state > INACTIVE && state < ACTIVE) {
		return true;
	}
	return false;
}

static void manager(void *context)
{
	sem_t *sema = (sem_t *)context;

	while (1) {
		sem_wait(sema);

		for (int i = 0; i < JOBPOOL_MAX_JOBS; i++) {
			if (is_pending(m.jobs[i].state)) {
				schedule_job(&m.jobs[i]);
			}
		}
	}
}

static struct job_desc *get_available_slot(void)
{
	for (int i = 0; i < JOBPOOL_MAX_JOBS; i++) {
		unsigned int index = atomic_fetch_add((int *)&m.job_index, 1)
			% JOBPOOL_MAX_JOBS;
		struct job_desc *p = &m.jobs[index];
		if (p->state == INACTIVE) {
			return p;
		}
	}

	return NULL;
}

bool jobpool_schedule(void (*job)(void *context), void *job_context)
{
	struct job_desc *p = get_available_slot();

	if (p == NULL) {
		return false;
	}

	int state = atomic_fetch_add(&p->state, 1);
	if (state != INACTIVE) {
		return false;
	}

	p->run = job;
	p->context = job_context;
	p->state = PENDING;

	sem_post_nointr(&m.sema);

	return true;
}

unsigned int jobpool_count(void)
{
	return (unsigned int)job_count(m.jobqueue);
}

bool jobpool_init(void)
{
	if (sem_init(&m.sema, 0, 0) != 0) {
		return false;
	}

	m.jobqueue = jobqueue_create(JOBPOOL_MAX_JOBS);
	if (m.jobqueue == NULL) {
		return false;
	}
	if (jobqueue_set_attr(m.jobqueue, &(jobqueue_attr_t) {
			.stack_size_bytes = JOBQUEUE_DEFAULT_STACK_SIZE,
			.priority = JOBQUEUE_DEFAULT_PRIORITY,
			.min_threads = 2,
			.max_threads = JOBPOOL_MAX_THREADS,
			}) != JOB_SUCCESS) {
		return false;
	}

	if (job_create_static(m.jobqueue, &m.manager, manager, &m.sema)
			!= JOB_SUCCESS) {
		return false;
	}
	if (job_schedule(m.jobqueue, &m.manager) != JOB_SUCCESS) {
		return false;
	}

	return true;
}
