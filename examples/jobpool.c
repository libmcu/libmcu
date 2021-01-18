#include "jobpool.h"
#include "libmcu/jobqueue.h"

#if !defined(JOBPOOL_MAX_JOBS)
#define JOBPOOL_MAX_JOBS		8
#endif
#if !defined(JOBPOOL_MAX_THREADS)
#define JOBPOOL_MAX_THREADS		3
#endif

static struct {
	jobqueue_t *jobqueue;
	job_t jobs[JOBPOOL_MAX_JOBS];
	unsigned int job_index;
} m;

bool jobpool_init(void)
{
	m.jobqueue = jobqueue_create(JOBPOOL_MAX_JOBS);
	if (m.jobqueue == NULL) {
		return false;
	}
	if (jobqueue_set_attr(m.jobqueue, &(jobqueue_attr_t) {
			.stack_size_bytes = JOBQUEUE_DEFAULT_STACK_SIZE,
			.priority = JOBQUEUE_DEFAULT_PRIORITY,
			.min_threads = 1,
			.max_threads = JOBPOOL_MAX_THREADS,
			}) != JOB_SUCCESS) {
		return false;
	}
	return true;
}

bool jobpool_schedule(void (*job)(void *context), void *job_context)
{
	if (job_init(m.jobqueue, &m.jobs[m.job_index], job, job_context)
			!= JOB_SUCCESS) {
		return false;
	}
	if (job_schedule(m.jobqueue, &m.jobs[m.job_index]) != JOB_SUCCESS) {
		return false;
	}

	m.job_index = (m.job_index + 1) % JOBPOOL_MAX_JOBS;

	return true;
}

unsigned int jobpool_count(void)
{
	return (unsigned int)job_count(m.jobqueue);
}
