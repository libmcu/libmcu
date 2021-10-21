COMPONENT_NAME = cli_commands

SRC_FILES = \
	../modules/cli/src/commands/cmd_exit.c \
	../modules/cli/src/commands/cmd_info.c \
	../modules/cli/src/commands/cmd_memdump.c

TEST_SRC_FILES = \
	src/cli/test_cli_commands.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/logging/include \
	../modules/cli/src \
	../modules/cli/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
