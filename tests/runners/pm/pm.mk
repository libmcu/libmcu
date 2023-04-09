# SPDX-License-Identifier: MIT

COMPONENT_NAME = PM

SRC_FILES = \
	../modules/pm/src/pm.c \

TEST_SRC_FILES = \
	src/pm/pm_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/pm/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS = -DUNITTEST
CPPUTEST_LDFLAGS = -lpthread

include runners/MakefileRunner
