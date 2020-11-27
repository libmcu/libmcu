COMPONENT_NAME = apptimer

SRC_FILES = \
	../src/apptimer.c

TEST_SRC_FILES = \
	src/test_apptimer.cpp

INCLUDE_DIRS += stubs

include test_runners/MakefileRunner.mk
