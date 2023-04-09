# SPDX-License-Identifier: MIT

COMPONENT_NAME = trace

SRC_FILES = \
	../modules/trace/src/trace.c \
	../modules/trace/src/trace_impl.c

TEST_SRC_FILES = \
	src/trace/trace_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/trace/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

CPPUTEST_CPPFLAGS =
MOCKS_SRC_DIRS =

include runners/MakefileRunner
