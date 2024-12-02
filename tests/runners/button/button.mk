# SPDX-License-Identifier: MIT

COMPONENT_NAME = Button

SRC_FILES = \
	../modules/button/src/button.c \
	../modules/button/src/button_overrides.c \

TEST_SRC_FILES = \
	src/button/button_test.cpp \
	src/button/button_delta_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs/overrides \
	../modules/button/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

CPPUTEST_CPPFLAGS = \
	-DBUTTON_MAX=3 \
	-DBUTTON_SAMPLING_PERIOD_MS=10 \
	-DBUTTON_DEBOUNCE_DURATION_MS=60 \
	-DBUTTON_REPEAT_DELAY_MS=300 \

include runners/MakefileRunner
