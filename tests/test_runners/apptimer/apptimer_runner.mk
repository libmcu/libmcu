COMPONENT_NAME = apptimer

SRC_FILES = \
	stubs/logging.c \
	../components/apptimer/src/apptimer.c

TEST_SRC_FILES = \
	src/apptimer/test_apptimer.cpp

INCLUDE_DIRS += \
	stubs \
	../components/logging/include \
	../components/apptimer/include

include test_runners/MakefileRunner.mk
