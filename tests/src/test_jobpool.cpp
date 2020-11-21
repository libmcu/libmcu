#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include <string.h>
#include "libmcu/jobpool.h"

#define MAX_JOBS	10

struct job_context {
	bool is_callback_called;
};

TEST_GROUP(JobPool) {
	jobpool_t *jobpool;
	job_t jobs[MAX_JOBS];
	job_context_t jobctx[MAX_JOBS];

	void setup(void) {
		mock().ignoreOtherCalls();

		jobpool = jobpool_create(100);
		jobpool_attr_t jobpool_attr = {
			.stack_size_bytes = 1024,
			.min_threads = -1, // to avoid infinite loop in jobpool_process()
			.max_threads = 1,
			.priority = 0,
		};
		jobpool_set_attr(jobpool, &jobpool_attr);

		memset(jobs, 0, sizeof(jobs));
		memset(jobctx, 0, sizeof(jobctx));
	}
	void teardown(void) {
		jobpool_destroy(jobpool);

		mock().checkExpectations();
		mock().clear();
	}

	static job_error_t callback(job_context_t *context) {
		context->is_callback_called = true;
		return JOB_SUCCESS;
	}
};

TEST(JobPool, stringify_ShouldReturnErrorString) {
	STRCMP_EQUAL("success", job_stringify_error(JOB_SUCCESS));
	STRCMP_EQUAL("retry", job_stringify_error(JOB_RETRY));
	STRCMP_EQUAL("invalid parameters", job_stringify_error(JOB_INVALID_PARAM));
	STRCMP_EQUAL("unknown error", job_stringify_error(JOB_ERROR));
}

TEST(JobPool, init_ShouldReturnParamError_WhenPoolOrJobIsNull) {
	LONGS_EQUAL(JOB_INVALID_PARAM, job_init(NULL, &jobs[0], NULL, NULL));
	LONGS_EQUAL(JOB_INVALID_PARAM, job_init(jobpool, NULL, NULL, NULL));
}

TEST(JobPool, init_ShouldReturnSuccess) {
	LONGS_EQUAL(JOB_SUCCESS, job_init(jobpool, &jobs[0], NULL, NULL));
}

TEST(JobPool, schedule_ShouldReturnParamError_WhenPoolOrJobIsNull) {
	job_init(jobpool, &jobs[0], NULL, NULL);

	LONGS_EQUAL(JOB_INVALID_PARAM, job_schedule(NULL, &jobs[0]));
	LONGS_EQUAL(JOB_INVALID_PARAM, job_schedule(jobpool, NULL));
}

TEST(JobPool, schedule_ShouldReturnSuccess) {
	job_init(jobpool, &jobs[0], callback, &jobctx[0]);

	mock().expectOneCall("pthread_attr_init");
	mock().expectOneCall("pthread_attr_setstacksize");
	mock().expectOneCall("pthread_attr_setdetachstate");
	mock().expectOneCall("pthread_create");

	LONGS_EQUAL(JOB_SUCCESS, job_schedule(jobpool, &jobs[0]));
	CHECK(jobctx[0].is_callback_called == true);
}

TEST(JobPool, delete_ShouldReturnInvalidParam_WhenNullPointersGiven) {
	LONGS_EQUAL(JOB_INVALID_PARAM, job_delete(NULL, &jobs[0]));
	LONGS_EQUAL(JOB_INVALID_PARAM, job_delete(jobpool, NULL));
}

TEST(JobPool, delete_ShouldReturnSuccess_WhenNoMatchingScheduledJob) {
	job_init(jobpool, &jobs[0], NULL, NULL);

	LONGS_EQUAL(0, job_count(jobpool));
	LONGS_EQUAL(JOB_SUCCESS, job_delete(jobpool, &jobs[0]));
	LONGS_EQUAL(0, job_count(jobpool));
}

TEST(JobPool, delete_ShouldReturnSuccess_WhenScheduledJobDeleted) {
	job_init(jobpool, &jobs[0], NULL, NULL);

	LONGS_EQUAL(0, job_count(jobpool));
	mock().expectOneCall("pthread_create").andReturnValue(-1);
	job_schedule(jobpool, &jobs[0]);
	LONGS_EQUAL(1, job_count(jobpool));

	LONGS_EQUAL(JOB_SUCCESS, job_delete(jobpool, &jobs[0]));
	LONGS_EQUAL(0, job_count(jobpool));
}

TEST(JobPool, count_ShouldReturnJobsScheduled) {
	mock().expectNCalls(MAX_JOBS, "pthread_create").andReturnValue(-1);

	for (int i = 0; i < MAX_JOBS; i++) {
		job_init(jobpool, &jobs[i], callback, &jobctx[i]);

		LONGS_EQUAL(i, job_count(jobpool));
		job_schedule(jobpool, &jobs[i]);
		CHECK_EQUAL(false, jobctx[i].is_callback_called);
	}

	LONGS_EQUAL(MAX_JOBS, job_count(jobpool));
}

TEST(JobPool, schedule_ShouldRunCallback_WhenCallbackRegistered) {
	mock().expectNCalls(MAX_JOBS, "pthread_create");

	for (int i = 0; i < MAX_JOBS; i++) {
		job_init(jobpool, &jobs[i], callback, &jobctx[i]);
		job_schedule(jobpool, &jobs[i]);
		CHECK_EQUAL(true, jobctx[i].is_callback_called);
	}
}

TEST(JobPool, Create_ShouldReturnNull_WhenSemInitFail) {
	mock().expectOneCall("sem_init").andReturnValue(-1);
	POINTERS_EQUAL(NULL, jobpool_create(1));
}

TEST(JobPool, Create_ShouldReturnNull_WhenAllocFail) {
	cpputest_malloc_set_out_of_memory();
	POINTERS_EQUAL(NULL, jobpool_create(1));
	cpputest_malloc_set_not_out_of_memory();
}

TEST(JobPool, set_attr_ShouldReturnInvalidParam_WhenNullPointersGiven) {
	LONGS_EQUAL(JOB_INVALID_PARAM, jobpool_set_attr(NULL, NULL));
	LONGS_EQUAL(JOB_INVALID_PARAM, jobpool_set_attr(jobpool, NULL));
	jobpool_attr_t attr = { 0, };
	LONGS_EQUAL(JOB_INVALID_PARAM, jobpool_set_attr(NULL, &attr));
}

TEST(JobPool, destroy_ShouldReturnInvalidParam_WhenNullPointerGiven) {
	LONGS_EQUAL(JOB_INVALID_PARAM, jobpool_destroy(NULL));
}
