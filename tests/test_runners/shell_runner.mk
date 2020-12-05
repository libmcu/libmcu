COMPONENT_NAME = shell

SRC_FILES = \
	../src/shell/shell.c \
	../src/shell/commands/commands.c \
	../src/shell/commands/cmd_exit.c \
	../src/shell/commands/cmd_help.c

TEST_SRC_FILES = \
	src/test_shell.cpp

INCLUDE_DIRS += \
	../src/shell

include test_runners/MakefileRunner.mk
