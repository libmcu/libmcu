COMPONENT_NAME = jobqueue

SRC_FILES = \
	stubs/logging.c \
	fakes/fake_pthread_mutex.c \
	mocks/mock_semaphore.c \
	mocks/mock_pthread.cpp \
	../components/jobqueue/src/jobqueue.c

TEST_SRC_FILES = \
	src/jobqueue/test_jobqueue.cpp

INCLUDE_DIRS += \
	stubs \
	../components/common/include/libmcu/posix \
	../components/logging/include \
	../components/jobqueue/include

include test_runners/MakefileRunner.mk
