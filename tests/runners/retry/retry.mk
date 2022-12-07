# SPDX-License-Identifier: MIT

COMPONENT_NAME = retry

SRC_FILES = \
	../modules/retry/src/retry.c

TEST_SRC_FILES = \
	src/retry/retry_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/logging/include \
	../modules/retry/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
