# SPDX-License-Identifier: MIT

COMPONENT_NAME = Cleanup

SRC_FILES = \
	../modules/cleanup/src/cleanup.c \

TEST_SRC_FILES = \
	src/cleanup/cleanup_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/cleanup/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS = -DUNITTEST

include runners/MakefileRunner
