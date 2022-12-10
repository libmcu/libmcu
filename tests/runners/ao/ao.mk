# SPDX-License-Identifier: MIT

COMPONENT_NAME = ao

SRC_FILES = \
	../modules/ao/src/ao.c \
	../modules/ao/src/ao_timer.c \
	../modules/ao/src/ao_overrides.c \
	../modules/common/src/bitops.c

TEST_SRC_FILES = \
	src/ao/ao_test.cpp \
	src/test_all.cpp \
	fakes/fake_semaphore_ios.c

INCLUDE_DIRS = \
	../modules/common/include \
	../modules/ao/include \
	$(CPPUTEST_HOME)/include \
	. \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS = -DUNITTEST
CPPUTEST_LDFLAGS = -lpthread

include MakefileRunner.mk
