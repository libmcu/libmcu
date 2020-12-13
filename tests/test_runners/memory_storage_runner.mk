COMPONENT_NAME = memory_storage

SRC_FILES = \
	../examples/memory_storage.c \
	../examples/ringbuf.c

TEST_SRC_FILES = \
	src/test_memory_storage.cpp

INCLUDE_DIRS += \
	../examples

include test_runners/MakefileRunner.mk
