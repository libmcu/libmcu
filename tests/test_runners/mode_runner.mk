COMPONENT_NAME = mode

SRC_FILES = \
	../src/mode.c \
	stubs/logging.c

TEST_SRC_FILES = \
	src/test_mode.cpp

INCLUDE_DIRS += stubs

include test_runners/MakefileRunner.mk
