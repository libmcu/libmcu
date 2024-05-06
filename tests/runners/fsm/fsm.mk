# SPDX-License-Identifier: MIT

COMPONENT_NAME = FSM

SRC_FILES = \
	../modules/fsm/src/fsm.c \

TEST_SRC_FILES = \
	src/fsm/fsm_test.cpp \
	stubs/logging.c \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/fsm/include \
	../modules/logging/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS = -DUNITTEST -include libmcu/logging.h -DFSM_DEBUG=debug
CPPUTEST_LDFLAGS = -lpthread

include runners/MakefileRunner
