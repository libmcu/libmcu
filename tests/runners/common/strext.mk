# SPDX-License-Identifier: MIT

COMPONENT_NAME = strext

SRC_FILES = \
	../modules/common/src/strext.c

TEST_SRC_FILES = \
	src/common/strext_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include runners/MakefileRunner
