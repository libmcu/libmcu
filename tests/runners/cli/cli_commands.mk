# SPDX-License-Identifier: MIT

COMPONENT_NAME = cli_commands

SRC_FILES = \
	../examples/cli/cmd_exit.c \
	../examples/cli/cmd_info.c \
	../examples/cli/cmd_memdump.c

TEST_SRC_FILES = \
	src/cli/cli_commands_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/logging/include \
	../examples/cli \
	../modules/cli/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
