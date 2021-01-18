COMPONENT_NAME = logging

SRC_FILES = \
	../examples/memory_storage.c \
	../components/common/src/ringbuf.c \
	../components/logging/src/logging.c

TEST_SRC_FILES = \
	src/logging/test_logging.cpp

INCLUDE_DIRS += \
	../examples \
	../components/logging/include

include test_runners/MakefileRunner.mk
