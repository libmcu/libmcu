COMPONENT_NAME = ringbuf

SRC_FILES = \
	../components/common/src/ringbuf.c \

TEST_SRC_FILES = \
	src/common/test_ringbuf.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
