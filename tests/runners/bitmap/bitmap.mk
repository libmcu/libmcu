COMPONENT_NAME = bitmap

SRC_FILES = \
	../modules/bitmap/src/bitmap.c

TEST_SRC_FILES = \
	src/bitmap/bitmap_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/bitmap/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
