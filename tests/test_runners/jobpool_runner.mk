COMPONENT_NAME = JobPool

SRC_FILES = \
	../src/jobpool.c

TEST_SRC_FILES = \
	src/test_jobpool.cpp

INCLUDE_DIRS += stubs ../include/posix
CPPUTEST_LDFLAGS += -lpthread

include test_runners/MakefileRunner.mk
