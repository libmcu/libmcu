COMPONENT_NAME = bitmap

SRC_FILES = \
	../src/bitmap.c

TEST_SRC_FILES = \
	src/test_bitmap.cpp

include test_runners/MakefileRunner.mk
