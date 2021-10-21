COMPONENT_NAME = shell_commands

SRC_FILES = \
	../modules/shell/src/commands/cmd_exit.c \
	../modules/shell/src/commands/cmd_info.c \
	../modules/shell/src/commands/cmd_memdump.c

TEST_SRC_FILES = \
	src/shell/test_shell_commands.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/logging/include \
	../modules/shell/src \
	../modules/shell/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
