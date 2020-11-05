COMPONENT_NAME = JobPool

SRC_FILES = \
	../src/jobpool.c \
	fakes/fake_pthread_mutex.c \
	mocks/mock_semaphore.c \
	mocks/mock_pthread.cpp

TEST_SRC_FILES = \
	src/test_jobpool.cpp

INCLUDE_DIRS += stubs ../include/posix

include test_runners/MakefileRunner.mk
