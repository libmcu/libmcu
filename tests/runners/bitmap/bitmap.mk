COMPONENT_NAME = bitmap

SRC_FILES = \
	../components/bitmap/src/bitmap.c

TEST_SRC_FILES = \
	src/bitmap/bitmap_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../components/bitmap/include \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
