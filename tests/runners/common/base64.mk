COMPONENT_NAME = base64

SRC_FILES = \
	../components/common/src/base64.c

TEST_SRC_FILES = \
	src/common/test_base64.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk