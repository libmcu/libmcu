COMPONENT_NAME = shell_commands

SRC_FILES = \
	../src/shell/commands/cmd_exit.c \
	../src/shell/commands/cmd_version.c

TEST_SRC_FILES = \
	src/shell/test_shell_commands.cpp

INCLUDE_DIRS += \
	../src/shell

include test_runners/MakefileRunner.mk
