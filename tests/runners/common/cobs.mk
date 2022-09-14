# SPDX-License-Identifier: MIT

COMPONENT_NAME = cobs

SRC_FILES = \
	../modules/common/src/cobs.c

TEST_SRC_FILES = \
	src/common/cobs_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
