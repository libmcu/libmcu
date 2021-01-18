COMPONENT_NAME = shell

SRC_FILES = \
	../components/shell/src/shell.c

TEST_SRC_FILES = \
	src/shell/test_shell.cpp

INCLUDE_DIRS += \
	../components/shell/src \
	../components/shell/include

include test_runners/MakefileRunner.mk
