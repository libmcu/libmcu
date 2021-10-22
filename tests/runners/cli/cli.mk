COMPONENT_NAME = cli

SRC_FILES = \
	../modules/cli/src/cli.c

TEST_SRC_FILES = \
	src/cli/cli_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs/overrides \
	../modules/cli/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
