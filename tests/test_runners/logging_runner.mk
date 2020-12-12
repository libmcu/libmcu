COMPONENT_NAME = logging

SRC_FILES = \
	../src/logging.c \
	stubs/memory_storage.c \
	stubs/ringbuf.c

TEST_SRC_FILES = \
	src/test_logging.cpp

include test_runners/MakefileRunner.mk
