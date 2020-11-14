COMPONENT_NAME = list

SRC_FILES = \
	stubs/empty.c

TEST_SRC_FILES = \
	src/test_list.cpp

include test_runners/MakefileRunner.mk
