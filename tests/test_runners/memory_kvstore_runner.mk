COMPONENT_NAME = memory_kvstore

SRC_FILES = \
	../examples/memory_kvstore.c

TEST_SRC_FILES = \
	src/test_memory_kvstore.cpp

INCLUDE_DIRS += \
	../examples

include test_runners/MakefileRunner.mk
