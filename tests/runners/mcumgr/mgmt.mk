# SPDX-License-Identifier: MIT

COMPONENT_NAME = mgmt

# mgmt_init stub is defined in the test file itself.
# A placeholder source is needed to satisfy the CppUTest archive step.
SRC_FILES = \
	stubs/mgmt.c \

TEST_SRC_FILES = \
	src/mcumgr/mgmt_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../interfaces/mcumgr/include \
	$(CPPUTEST_HOME)/include \
	. \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =
CPPUTEST_LDFLAGS =

include runners/MakefileRunner
