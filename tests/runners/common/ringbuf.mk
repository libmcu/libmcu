COMPONENT_NAME = ringbuf

SRC_FILES = \
	stubs/bitops.c \
	../components/common/src/ringbuf.c \

TEST_SRC_FILES = \
	src/common/ringbuf_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
