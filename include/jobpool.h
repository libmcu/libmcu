#ifndef JOBPOOL_H
#define JOBPOOL_H 1

#include <stddef.h>
#include <stdint.h>

typedef enum {
	JOB_SUCCESS				= 0,
	JOB_RETRY,
	JOB_ERROR,
	JOB_ERROR_PARAM,
} job_error_t;

typedef union {
#if defined(__WORDSIZE) && __WORDSIZE == 64
	char _size[48];
#else
	char _size[24];
#endif
	long _align;
} job_t;

typedef struct job_context job_context_t;

typedef struct jobpool jobpool_t;
typedef struct jobpool_attr {
	size_t stack_size_bytes;
	uint8_t min_threads;
	uint8_t max_threads;
	int8_t priority;
} jobpool_attr_t;

typedef job_error_t (*job_callback_t)(job_context_t *context);

jobpool_t *jobpool_create(unsigned int max_concurrent_jobs);
job_error_t jobpool_set_attr(jobpool_t *pool, const jobpool_attr_t *attr);
job_error_t jobpool_destroy(jobpool_t *pool);

job_error_t job_init(jobpool_t *pool, job_t *job, job_callback_t callback, job_context_t *context);
job_error_t job_schedule(jobpool_t *pool, job_t *job);
job_error_t job_delete(jobpool_t *pool, job_t *job);

uint8_t job_count(jobpool_t *pool);
const char *job_stringify_error(job_error_t error_code);

#endif /* JOBPOOL_H */
