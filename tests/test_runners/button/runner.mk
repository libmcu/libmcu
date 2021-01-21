COMPONENT_NAME = button

SRC_FILES = \
	stubs/logging.c \
	../components/button/src/button.c

TEST_SRC_FILES = \
	src/button/test_button.cpp

INCLUDE_DIRS += \
	../components/logging/include \
	../components/button/include

CPPUTEST_CPPFLAGS += -DBUTTON_MAX=3

include test_runners/MakefileRunner.mk
