COMPONENT_NAME = jobqueue_integration

SRC_FILES = \
	../src/jobqueue.c \
	stubs/logging.c

TEST_SRC_FILES = \
	src/test_jobqueue_integration.cpp

INCLUDE_DIRS += stubs ../include/libmcu/posix
CPPUTEST_LDFLAGS += -lpthread

include test_runners/MakefileRunner.mk
