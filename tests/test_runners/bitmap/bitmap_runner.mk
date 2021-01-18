COMPONENT_NAME = bitmap

SRC_FILES = \
	../components/bitmap/src/bitmap.c

TEST_SRC_FILES = \
	src/bitmap/test_bitmap.cpp

INCLUDE_DIRS += \
	../components/bitmap/include

include test_runners/MakefileRunner.mk
