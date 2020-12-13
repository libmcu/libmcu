COMPONENT_NAME = logging

SRC_FILES = \
	../src/logging.c \
	../examples/memory_storage.c \
	../examples/ringbuf.c

TEST_SRC_FILES = \
	src/test_logging.cpp

include test_runners/MakefileRunner.mk
