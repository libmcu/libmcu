/*
 * SPDX-FileCopyrightText: 2020 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"

#include <pthread.h>
#include "libmcu/jobqueue.h"

typedef struct job_context {
	bool is_callback_called;
	pthread_t thread;
} job_context_t;

TEST_GROUP(JobPool_Integration) {
	jobqueue_t *jobqueue;

	void setup(void) {
		jobqueue = jobqueue_create(100);

		pthread_attr_t attr;
		size_t stacksize;
		pthread_attr_init(&attr);
		pthread_attr_getstacksize(&attr, &stacksize);
		jobqueue_attr_t jobqueue_attr = {
			.stack_size_bytes = stacksize,
			.min_threads = 0,
			.max_threads = 10,
			.priority = 0,
		};
		jobqueue_set_attr(jobqueue, &jobqueue_attr);
	}
	void teardown(void) {
		jobqueue_destroy(jobqueue);
	}

	void wait_callback_to_be_called(job_context_t *ctx) {
		while (!ctx->is_callback_called);
	}

	static void callback(void *context) {
		job_context_t *p = (job_context_t *)context;
		p->is_callback_called = true;
		p->thread = pthread_self();
	}
};

#ifdef __APPLE__
IGNORE_TEST(JobPool_Integration,
		IgnoreTest_WhenSemaphoreNotWorkAsExpectedOnMacOSX) {
}
#else
TEST(JobPool_Integration, stringify_ShouldReturnErrorString) {
	STRCMP_EQUAL("success", job_stringify_error(JOB_SUCCESS));
	STRCMP_EQUAL("unknown error", job_stringify_error(JOB_ERROR));
}

TEST(JobPool_Integration, init_ShouldReturnParamError_WhenPoolOrJobIsNull) {
	job_t job;
	CHECK_EQUAL(JOB_INVALID_PARAM, job_init(NULL, &job, NULL, NULL));
	CHECK_EQUAL(JOB_INVALID_PARAM, job_init(jobqueue, NULL, NULL, NULL));
}

TEST(JobPool_Integration, init_ShouldReturnSuccess) {
	job_t job;
	CHECK_EQUAL(JOB_SUCCESS, job_init(jobqueue, &job, NULL, NULL));
}

TEST(JobPool_Integration, schedule_ShouldReturnParamError_WhenPoolOrJobIsNull) {
	job_t job;
	job_init(jobqueue, &job, NULL, NULL);

	CHECK_EQUAL(JOB_INVALID_PARAM, job_schedule(NULL, &job));
	CHECK_EQUAL(JOB_INVALID_PARAM, job_schedule(jobqueue, NULL));
}

TEST(JobPool_Integration, schedule_ShouldReturnSuccess) {
	job_t job;
	job_context_t jobctx = { 0, 0, };
	job_init(jobqueue, &job, callback, &jobctx);

	CHECK_EQUAL(JOB_SUCCESS, job_schedule(jobqueue, &job));

	wait_callback_to_be_called(&jobctx);
	pthread_join(jobctx.thread, NULL);
}

TEST(JobPool_Integration, delete_ShouldReturnSuccess_WhenNoMatchingScheduledJob) {
	job_t job;
	job_init(jobqueue, &job, NULL, NULL);

	CHECK_EQUAL(0, job_count(jobqueue));
	CHECK_EQUAL(JOB_SUCCESS, job_delete(jobqueue, &job));
	CHECK_EQUAL(0, job_count(jobqueue));
}

TEST(JobPool_Integration, count_ShouldReturnJobsScheduled) {
	job_t job;
	job_context_t jobctx = { 0, 0, };
	job_init(jobqueue, &job, callback, &jobctx);

	CHECK_EQUAL(0, job_count(jobqueue));
	job_schedule(jobqueue, &job);
	CHECK_EQUAL(1, job_count(jobqueue));

	wait_callback_to_be_called(&jobctx);
	pthread_join(jobctx.thread, NULL);
}

TEST(JobPool_Integration, schedule_ShouldRunCallback_WhenCallbackRegistered) {
	job_t job1, job2, job3;
	job_context_t jobctx1 = { 0, 0, };
	job_context_t jobctx2 = { 0, 0, };
	job_context_t jobctx3 = { 0, 0, };
	job_init(jobqueue, &job1, callback, &jobctx1);
	job_init(jobqueue, &job2, callback, &jobctx2);
	job_init(jobqueue, &job3, callback, &jobctx3);
	job_schedule(jobqueue, &job1);
	job_schedule(jobqueue, &job2);
	job_schedule(jobqueue, &job3);
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
