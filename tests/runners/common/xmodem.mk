# SPDX-License-Identifier: MIT

COMPONENT_NAME = XMODEM

SRC_FILES = \
	../modules/common/src/xmodem.c \
	../modules/common/src/crc16.c \

TEST_SRC_FILES = \
	src/common/xmodem_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include runners/MakefileRunner
