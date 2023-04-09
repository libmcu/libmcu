# SPDX-License-Identifier: MIT

COMPONENT_NAME = jobqueue

SRC_FILES = \
	stubs/logging.c \
	fakes/fake_pthread_mutex.c \
	mocks/mock_semaphore.c \
	mocks/mock_pthread.cpp \
	../modules/jobqueue/src/jobqueue.c

TEST_SRC_FILES = \
	src/jobqueue/jobqueue_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs \
	../modules/common/include/libmcu/posix \
	../modules/common/include \
	../modules/logging/include \
	../modules/jobqueue/include \
	$(CPPUTEST_HOME)/include \
	. \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS = -DUNITTEST

include runners/MakefileRunner
