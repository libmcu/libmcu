COMPONENT_NAME = mode

SRC_FILES = \
	stubs/logging.c \
	../components/mode/src/mode.c

TEST_SRC_FILES = \
	src/mode/test_mode.cpp

INCLUDE_DIRS += \
	stubs \
	../components/logging/include \
	../components/mode/include

include test_runners/MakefileRunner.mk
