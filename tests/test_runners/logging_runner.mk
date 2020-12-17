COMPONENT_NAME = logging

SRC_FILES = \
	../src/logging.c \
	../src/ringbuf.c \
	../examples/logging/memory_storage.c

TEST_SRC_FILES = \
	src/test_logging.cpp

INCLUDE_DIRS += \
	../examples \
	../examples/logging

include test_runners/MakefileRunner.mk
