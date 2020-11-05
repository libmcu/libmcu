#include "CppUTest/TestHarness.h"

#include <pthread.h>
#include "jobpool.h"

struct job_context {
	bool is_callback_called;
	pthread_t thread;
};

TEST_GROUP(JobPool_Integration) {
	jobpool_t *jobpool;

	void setup(void) {
		jobpool = jobpool_create(100);

		pthread_attr_t attr;
		size_t stacksize;
		pthread_attr_init(&attr);
		pthread_attr_getstacksize(&attr, &stacksize);
		jobpool_attr_t jobpool_attr = {
			.stack_size_bytes = stacksize,
			.min_threads = 0,
			.max_threads = 10,
			.priority = 0,
		};
		jobpool_set_attr(jobpool, &jobpool_attr);
	}
	void teardown(void) {
		jobpool_destroy(jobpool);
	}

	void wait_callback_to_be_called(job_context_t *ctx) {
		while (!ctx->is_callback_called);
	}

	static job_error_t callback(job_context_t *context) {
		context->is_callback_called = true;
		context->thread = pthread_self();
		return JOB_SUCCESS;
	}
};

#ifdef __APPLE__
IGNORE_TEST(JobPool_Integration,
		IgnoreTest_WhenSemaphoreNotWorkAsExpectedOnMacOSX) {
}
#else
TEST(JobPool_Integration, stringify_ShouldReturnErrorString) {
	STRCMP_EQUAL("success", job_stringify_error(JOB_SUCCESS));
	STRCMP_EQUAL("retry", job_stringify_error(JOB_RETRY));
	STRCMP_EQUAL("unknown error", job_stringify_error(JOB_ERROR));
}

TEST(JobPool_Integration, init_ShouldReturnParamError_WhenPoolOrJobIsNull) {
	job_t job;
	CHECK_EQUAL(JOB_INVALID_PARAM, job_init(NULL, &job, NULL, NULL));
	CHECK_EQUAL(JOB_INVALID_PARAM, job_init(jobpool, NULL, NULL, NULL));
}

TEST(JobPool_Integration, init_ShouldReturnSuccess) {
	job_t job;
	CHECK_EQUAL(JOB_SUCCESS, job_init(jobpool, &job, NULL, NULL));
}

TEST(JobPool_Integration, schedule_ShouldReturnParamError_WhenPoolOrJobIsNull) {
	job_t job;
	job_init(jobpool, &job, NULL, NULL);

	CHECK_EQUAL(JOB_INVALID_PARAM, job_schedule(NULL, &job));
	CHECK_EQUAL(JOB_INVALID_PARAM, job_schedule(jobpool, NULL));
}

TEST(JobPool_Integration, schedule_ShouldReturnSuccess) {
	job_t job;
	job_context_t jobctx = { 0, 0, };
	job_init(jobpool, &job, callback, &jobctx);

	CHECK_EQUAL(JOB_SUCCESS, job_schedule(jobpool, &job));

	wait_callback_to_be_called(&jobctx);
	pthread_join(jobctx.thread, NULL);
}

TEST(JobPool_Integration, delete_ShouldReturnSuccess_WhenNoMatchingScheduledJob) {
	job_t job;
	job_init(jobpool, &job, NULL, NULL);

	CHECK_EQUAL(0, job_count(jobpool));
	CHECK_EQUAL(JOB_SUCCESS, job_delete(jobpool, &job));
	CHECK_EQUAL(0, job_count(jobpool));
}

TEST(JobPool_Integration, count_ShouldReturnJobsScheduled) {
	job_t job;
	job_context_t jobctx = { 0, 0, };
	job_init(jobpool, &job, callback, &jobctx);

	CHECK_EQUAL(0, job_count(jobpool));
	job_schedule(jobpool, &job);
	CHECK_EQUAL(1, job_count(jobpool));

	wait_callback_to_be_called(&jobctx);
	pthread_join(jobctx.thread, NULL);
}

TEST(JobPool_Integration, schedule_ShouldRunCallback_WhenCallbackRegistered) {
	job_t job1, job2, job3;
	job_context_t jobctx1 = { 0, 0, };
	job_context_t jobctx2 = { 0, 0, };
	job_context_t jobctx3 = { 0, 0, };
	job_init(jobpool, &job1, callback, &jobctx1);
	job_init(jobpool, &job2, callback, &jobctx2);
	job_init(jobpool, &job3, callback, &jobctx3);
	job_schedule(jobpool, &job1);
	job_schedule(jobpool, &job2);
	job_schedule(jobpool, &job3);
	wait_callback_to_be_called(&jobctx1);
	wait_callback_to_be_called(&jobctx2);
	wait_callback_to_be_called(&jobctx3);

	CHECK_EQUAL(true, jobctx1.is_callback_called);
	CHECK_EQUAL(true, jobctx2.is_callback_called);
	CHECK_EQUAL(true, jobctx3.is_callback_called);

	pthread_join(jobctx1.thread, NULL);
	pthread_join(jobctx2.thread, NULL);
	pthread_join(jobctx3.thread, NULL);
}
#endif
