# SPDX-License-Identifier: MIT

COMPONENT_NAME = RateLim

SRC_FILES = \
	../modules/ratelim/src/ratelim.c \

TEST_SRC_FILES = \
	src/ratelim/ratelim_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/ratelim/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include runners/MakefileRunner
