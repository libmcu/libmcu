#ifndef LIBMCU_JOBPOOL_H
#define LIBMCU_JOBPOOL_H 202012L

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

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
} job_t;

typedef struct job_context job_context_t;

typedef struct jobpool jobpool_t;
typedef struct jobpool_attr {
	size_t stack_size_bytes;
	int8_t min_threads;
	uint8_t max_threads;
	int8_t priority;
} jobpool_attr_t;

typedef void (*job_callback_t)(job_context_t *context);

jobpool_t *jobpool_create(unsigned int max_concurrent_jobs);
job_error_t jobpool_set_attr(jobpool_t *pool, const jobpool_attr_t *attr);
job_error_t jobpool_destroy(jobpool_t *pool);

job_error_t job_init(jobpool_t *pool, job_t *job, job_callback_t callback, job_context_t *context);
job_error_t job_schedule(jobpool_t *pool, job_t *job);
job_error_t job_delete(jobpool_t *pool, job_t *job);

uint8_t job_count(jobpool_t *pool);
const char *job_stringify_error(job_error_t error_code);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_JOBPOOL_H */
