COMPONENT_NAME = shell

SRC_FILES = \
	../src/shell/shell.c

TEST_SRC_FILES = \
	src/test_shell.cpp

include test_runners/MakefileRunner.mk
