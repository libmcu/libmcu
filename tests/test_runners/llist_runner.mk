COMPONENT_NAME = llist

SRC_FILES = \
	stubs/empty.c

TEST_SRC_FILES = \
	src/test_llist.cpp

include test_runners/MakefileRunner.mk
