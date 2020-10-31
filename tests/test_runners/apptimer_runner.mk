COMPONENT_NAME = AppTimer

SRC_FILES = \
	../ports/esp-idf/apptimer.c \
	spies/apptimer_spy.c

TEST_SRC_FILES = \
	src/test_apptimer.cpp

INCLUDE_DIRS += stubs

include test_runners/MakefileRunner.mk
