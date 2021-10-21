COMPONENT_NAME = hexdump

SRC_FILES = \
	../modules/common/src/hexdump.c \

TEST_SRC_FILES = \
	src/common/hexdump_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
