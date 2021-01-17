COMPONENT_NAME = ringbuf

SRC_FILES = ../components/common/src/ringbuf.c

TEST_SRC_FILES = src/common/test_ringbuf.cpp

include test_runners/MakefileRunner.mk
