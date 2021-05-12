#ifndef LIBMCU_JOBQUEUE_H
#define LIBMCU_JOBQUEUE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#if !defined(JOBQUEUE_DEFAULT_STACK_SIZE)
#define JOBQUEUE_DEFAULT_STACK_SIZE		3072
#endif
#if !defined(JOBQUEUE_DEFAULT_MIN_THREADS)
#define JOBQUEUE_DEFAULT_MIN_THREADS		1
#endif
#if !defined(JOBQUEUE_DEFAULT_MAX_THREADS)
#define JOBQUEUE_DEFAULT_MAX_THREADS		1
#endif
#if !defined(JOBQUEUE_DEFAULT_PRIORITY)
#define JOBQUEUE_DEFAULT_PRIORITY		5
#endif

#if !defined(JOBQUEUE_DEBUG)
#define JOBQUEUE_DEBUG(...)
#endif

typedef enum {
	JOB_SUCCESS				= 0,
	JOB_INVALID_PARAM,
	JOB_FULL,
	JOB_ERROR,
} job_error_t;

typedef union {
#if defined(__amd64__) || defined(__x86_64__) || defined(__aarch64__) \
	|| defined(__ia64__) || defined(__ppc64__)
	char _size[48];
#else // 32-bit
	char _size[24];
#endif
	long _align;
} job_static_t;

typedef job_static_t * job_t;
typedef struct jobqueue * jobqueue_t;

typedef struct jobqueue_attr {
	size_t stack_size_bytes;
	int8_t min_threads;
	uint8_t max_threads;
	int8_t priority;
} jobqueue_attr_t;

typedef void (*job_callback_t)(void *context);

jobqueue_t jobqueue_create(uint8_t max_concurrent_jobs);
job_error_t jobqueue_set_attr(jobqueue_t pool, const jobqueue_attr_t *attr);
job_error_t jobqueue_destroy(jobqueue_t pool);

job_error_t job_create_static(jobqueue_t pool,
		job_t job, job_callback_t callback, void *context);
job_error_t job_schedule(jobqueue_t pool, job_t job);
job_error_t job_deschedule(jobqueue_t pool, job_t job);

uint8_t job_count(jobqueue_t pool);
const char *job_stringify_error(job_error_t error_code);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_JOBQUEUE_H */
