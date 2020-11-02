COMPONENT_NAME = RunMode

SRC_FILES = \
	../src/mode.c

TEST_SRC_FILES = \
	src/test_mode.cpp

INCLUDE_DIRS += stubs

include test_runners/MakefileRunner.mk
