# SPDX-License-Identifier: MIT

COMPONENT_NAME = CRC16

SRC_FILES = \
	../modules/common/src/crc16.c \

TEST_SRC_FILES = \
	src/common/crc16_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include runners/MakefileRunner
