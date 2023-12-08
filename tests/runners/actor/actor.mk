# SPDX-License-Identifier: MIT

COMPONENT_NAME = actor

SRC_FILES = \
	../modules/actor/src/actor.c \

TEST_SRC_FILES = \
	src/actor/actor_test.cpp \
	stubs/logging.cpp \
	src/test_all.cpp \
	fakes/fake_semaphore_ios.c

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
