# SPDX-License-Identifier: MIT

COMPONENT_NAME = actor_timer

SRC_FILES = \
	../modules/actor/src/actor_timer.c \
	../modules/actor/src/actor.c \

TEST_SRC_FILES = \
	src/actor/actor_timer_test.cpp \
	stubs/logging.cpp \
	src/test_all.cpp

ifeq ($(shell uname), Darwin)
TEST_SRC_FILES += fakes/fake_semaphore_ios.c
endif

INCLUDE_DIRS = \
	../modules/common/include/libmcu/posix \
	../modules/common/include \
	../modules/actor/include \
	../modules/logging/include \
	$(CPPUTEST_HOME)/include \
	. \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS = -DUNITTEST -include libmcu/logging.h \
		    -DACTOR_DEBUG=debug -DACTOR_INFO=info -DACTOR_WARN=warn
CPPUTEST_LDFLAGS = -lpthread

include runners/MakefileRunner
