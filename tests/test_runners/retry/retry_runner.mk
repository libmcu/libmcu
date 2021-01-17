COMPONENT_NAME = retry

SRC_FILES = \
	stubs/logging.c \
	../components/retry/src/retry.c

TEST_SRC_FILES = \
	src/retry/test_retry.cpp

INCLUDE_DIRS += \
	../components/logging/include \
	../components/retry/include

include test_runners/MakefileRunner.mk
