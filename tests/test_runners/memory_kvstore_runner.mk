COMPONENT_NAME = memory_kvstore

SRC_FILES = \
	../src/memory_kvstore.c

TEST_SRC_FILES = \
	src/test_memory_kvstore.cpp

include test_runners/MakefileRunner.mk
