COMPONENT_NAME = list

SRC_FILES = \
	stubs/bitops.c

TEST_SRC_FILES = \
	src/common/test_list.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
