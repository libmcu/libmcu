COMPONENT_NAME = shell

SRC_FILES = \
	../src/shell/shell.c

TEST_SRC_FILES = \
	src/shell/test_shell.cpp

INCLUDE_DIRS += \
	../src/shell

include test_runners/MakefileRunner.mk
