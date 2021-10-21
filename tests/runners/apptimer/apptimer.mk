COMPONENT_NAME = apptimer

SRC_FILES = \
	stubs/bitops.c \
	../modules/apptimer/src/apptimer.c \

TEST_SRC_FILES = \
	src/apptimer/apptimer_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	stubs/overrides \
	../modules/apptimer/include \
	../modules/common/include \
	$(CPPUTEST_HOME)/include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include MakefileRunner.mk
