COMPONENT_NAME = retry

SRC_FILES = \
	../src/retry.c \
	stubs/logging.c

TEST_SRC_FILES = \
	src/test_retry.cpp

include test_runners/MakefileRunner.mk
