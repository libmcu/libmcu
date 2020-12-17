COMPONENT_NAME = ringbuf

SRC_FILES = ../src/ringbuf.c

TEST_SRC_FILES = src/test_ringbuf.cpp

INCLUDE_DIRS += ../examples

include test_runners/MakefileRunner.mk
