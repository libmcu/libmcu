COMPONENT_NAME = mode

SRC_FILES = \
	stubs/logging.c \
	../components/mode/src/mode.c

TEST_SRC_FILES = \
	src/mode/test_mode.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs \
	../components/logging/include \
	../components/mode/include \
	../components/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
