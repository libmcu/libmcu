COMPONENT_NAME = logging

SRC_FILES = \
	../src/logging.c \
	../examples/logging/memory_storage.c \
	../examples/ringbuf.c

TEST_SRC_FILES = \
	src/test_logging.cpp

INCLUDE_DIRS += \
	../examples \
	../examples/logging

include test_runners/MakefileRunner.mk
