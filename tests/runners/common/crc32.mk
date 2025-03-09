# SPDX-License-Identifier: MIT

COMPONENT_NAME = CRC32

SRC_FILES = \
	../modules/common/src/crc32.c \

TEST_SRC_FILES = \
	src/common/crc32_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include runners/MakefileRunner
