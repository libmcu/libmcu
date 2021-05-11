COMPONENT_NAME = shell

SRC_FILES = \
	../components/shell/src/shell.c

TEST_SRC_FILES = \
	src/shell/test_shell.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs/overrides \
	../components/shell/src \
	../components/shell/include \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
