COMPONENT_NAME = jobpool

SRC_FILES = \
	../src/jobpool.c \
	fakes/fake_pthread_mutex.c \
	mocks/mock_semaphore.c \
	mocks/mock_pthread.cpp

TEST_SRC_FILES = \
	src/test_jobpool.cpp

INCLUDE_DIRS += stubs ../include/libmcu/posix

include test_runners/MakefileRunner.mk
