# SPDX-License-Identifier: MIT

COMPONENT_NAME = MessageQueue

SRC_FILES = \
	../modules/common/src/msgq.c \
	../modules/common/src/ringbuf.c \
	stubs/bitops.c \

TEST_SRC_FILES = \
	src/common/msgq_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include runners/MakefileRunner
