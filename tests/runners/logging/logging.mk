COMPONENT_NAME = logging

SRC_FILES = \
	stubs/bitops.c \
	../examples/memory_storage.c \
	../components/common/src/ringbuf.c \
	../components/logging/src/logging.c

TEST_SRC_FILES = \
	src/logging/test_logging.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../examples \
	../components/logging/include \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
