COMPONENT_NAME = jobqueue

SRC_FILES = \
	../src/jobqueue.c \
	fakes/fake_pthread_mutex.c \
	mocks/mock_semaphore.c \
	mocks/mock_pthread.cpp \
	stubs/logging.c

TEST_SRC_FILES = \
	src/test_jobqueue.cpp

INCLUDE_DIRS += stubs ../include/libmcu/posix

include test_runners/MakefileRunner.mk
